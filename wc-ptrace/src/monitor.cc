/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-08-03 17:45:47
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-13 14:47:13
 * @FilePath: /wc-intercept/wc-ptrace/src/monitor.cc
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include "elf.h"
#include "target.h"
#include "monitor.h"
#include "log.h"
#include "config.h"

#include <sys/syscall.h>
#include <sys/user.h>
#include <sys/types.h>

static wc::Logger::ptr logger = WC_LOG_NAME("intercept");

namespace wc {

Monitor::Monitor(Config::ptr config, TraceeElf::ptr elf, pid_t pid) {
    // 获取加载地址
    get_load_address(pid);
    m_pid = pid;
    if (m_load_address == 0x0) {
        std::logic_error("Monitor initialize error!");
    }

    // 设置配置文件路径，配置文件路径从进程启动就不可变
    m_config_path = config->m_conf_path;

    // 打开后端处理逻辑
    open_backend_library(config->get_string(CONFIG_LIBRARY));

    // 初始化要拦截的函数
    initial_funcs(config->m_values, elf);

    // 初始化要拦截的系统调用
    initial_syscalls(config->m_values);
}

Monitor::~Monitor() {
    m_syscalls.clear();
    m_funcs.clear();
    m_bps.clear();
    if (m_library) {
        dlclose(m_library);
    }
}

void Monitor::open_backend_library(const std::string& library_path) {
    if (m_library) {
        dlclose(m_library);
    }
    m_library = dlopen(library_path.c_str(), RTLD_LAZY);
    if (!m_library) {
        WC_LOG_ERROR(logger) << "[Monitor::open_backend_library] Can't open backend library (" << library_path
                             << ")!" << std::endl;
    }
}

backend_handle Monitor::get_backend(const std::string& backend_name) {
    if ( backend_name.empty() || !m_library ) {
        return nullptr;
    }
    backend_handle handle = (backend_handle)dlsym(m_library, backend_name.c_str());
    if (handle == NULL) {
        WC_LOG_ERROR(logger) << "[Monitor::get_backend] Can't found function handle " << backend_name
                             << "errno is " << dlerror() << std::endl;
    }
    return handle;
}

Function::ptr Monitor::get_func(TraceeElf::ptr elf, const std::string& symbol_name) {
    /* 先从动态表中查找符号 */
    bool is_dyn = true;
    int index = elf->search_symbol(DYNSYM, symbol_name);
    if (index == -1) {
        index = elf->search_symbol(SYMTAB, symbol_name);
        if (index == -1) {
            WC_LOG_ERROR(logger) << "Can't found symbol (" << symbol_name << ") "
                                 << "in ELF (" << elf->get_name() << ")." << std::endl;
            return nullptr;
        }
        is_dyn = false;
    }
    std::string table_name = is_dyn ? DYNSYM : SYMTAB;
    GElf_Sym *sym = elf->get_sym(table_name, index);
    if (!sym) {
        WC_LOG_ERROR(logger) << "Can't get symbol pointer(" << symbol_name << ") "
                                << "in ELF (" << elf->get_name() << ")." << std::endl;
        return nullptr;
    }
    if (!TraceeElf::is_function(sym->st_info)) {
        WC_LOG_ERROR(logger) << "symbol (" << symbol_name << ") is not a "
                                << "function!" << std::endl;
        return nullptr;
    }

    uint8_t flag = is_dyn ? FUNC_DYN : FUNC_NORMAL;
    flag = elf->is_bind_now() ? flag : FUNC_DELAY;
    Function::ptr func(new Function(symbol_name, flag, sym->st_value+m_load_address));

    if (is_dyn) {
        int index_rela = elf->search_rela_plt(index);
        if (index == -1) {
            WC_LOG_ERROR(logger) << "Can't get symbol rela index(" << symbol_name << ") "
                                    << "in ELF (" << elf->get_name() << ")." << std::endl;
            return nullptr;
        }
        GElf_Rela *rela = elf->get_sym_in_relaplt(index_rela);
        func->m_readdress = rela->r_offset + m_load_address;
    }
    return func;
}

void Monitor::entry(TraceeElf::ptr elf) {
    /*
       If the current program is being ptraced, a SIGTRAP signal is sent
       to it after a successful execve() by (execve(2) — Linux manual page)
       所以我们可知execve执行成功后，如果发现被ptraced，会发送SIGTRAP，给父进程中
       的waitpid(如果有的话)，然而经过我们验证，当execve执行完之后，新进程的各个段
       已经成功加载了，但是对于共享库函数来说，还未进行重定向，.got段中依旧是初始值
       所以如果需要拦截共享库符号，那么需要在entry处设置断点
    */
    WC_LOG_INFO(logger) << "load address is: " << std::hex << m_load_address << std::endl;
    if (!m_funcs.empty()) {
        uint64_t entry_address = elf->get_entry();
        BreakPoint::ptr entry_bp(new BreakPoint(
            BP_ENTRY,
            m_load_address+elf->get_entry(),
            m_pid,
            nullptr,
            nullptr
        ));
        m_bp_entry = entry_bp;
        m_bps.insert({m_load_address+elf->get_entry(), entry_bp});
        entry_bp->enable();
    }

}

void Monitor::wait(Config::ptr config, TraceeElf::ptr elf, bool& flag_update_1, bool& flag_update_2) {
    WC_LOG_ERROR(logger) << "funcs number: " << m_funcs.size() << std::endl;
    int status;
    for (;;) {
        if (ptrace(PTRACE_SYSCALL, m_pid, 0, 0) == -1){
            WC_LOG_ERROR(logger) << "[Monitor::wait] PTRACE_SYSCALL failed int syscall-enter-stop, will exit! "
                                 << "errno: " << strerror(errno) << std::endl;
            break;
        }
        if (waitpid(m_pid, &status, 0) == -1) {
            WC_LOG_ERROR(logger) << "[Monitor::wait] waitpid failed int syscall-enter-stop, will exit! "
                                 << "errno: " << strerror(errno) << std::endl;
            break;
        }
        /* 被跟踪进程非正常终止 */
        if (WIFSIGNALED(status)) {
            WC_LOG_ERROR(logger) << "[Monitor::wait]: tracee terminated abnormally " 
                                 << elf->get_name() << " pid:  " << m_pid
                                 << " status: " << WTERMSIG(status) << std::endl;
            break;
        }
        /* 被跟踪进程正常终止 */
        else if (WIFEXITED(status)) {
            WC_LOG_ERROR(logger) << "[Monitor::wait]: tracee normally exited " 
                                 << elf->get_name() << " pid:  " << m_pid
                                 << " status: " << WTERMSIG(status) << std::endl;
            break;
        }
        /* SIGTRAP */
        else if (WIFSTOPPED(status) && WSTOPSIG(status) == 0x5) {
            struct user_regs_struct regs;
            if (ptrace(PTRACE_GETREGS, m_pid, 0, &regs) == -1) {
                WC_LOG_ERROR(logger) << "[Monitor::wait]: Get regs failed! erron: "
                                     << strerror(errno) << std::endl;
                continue;
            }
            // 非拦截的SIGTRAP，可能是被跟踪程序主动抛出，忽略
            long bp_address = regs.rip-1;
            if (m_bps.find(bp_address) == m_bps.end()) {
                WC_LOG_ERROR(logger) << "[Monitor::wait] Unknown breakpoint! " << bp_address << std::endl;
                continue;
            }
            WC_LOG_DEBUG(logger) << "breakpoint trigger, breakpoint address: (" << bp_address
                                 << ") now breakpoint number: " << m_bps.size() << std::endl; 
            
            {
                BreakPoint::ptr bp = m_bps[bp_address].lock();
                trigger(bp, &regs);
            }
            if (flag_update_1) {
                update_1(config, elf);
                flag_update_1 = false;
            }
            if (flag_update_2) {
                update_2(config, elf);
                flag_update_2 = false;
            }
            continue;
        }
        
        /* syscall-enter-stop */
        else if (WIFSTOPPED(status) && WSTOPSIG(status) == 0x85) {
            struct user_regs_struct regs;
            if (ptrace(PTRACE_GETREGS, m_pid, 0, &regs) == -1) {
                WC_LOG_ERROR(logger) << "[Monitor::wait]: Get regs failed! erron: "
                                     << strerror(errno) << std::endl;
                continue;
            }
            if (m_syscalls.find(regs.orig_rax) != m_syscalls.end()) {
                if (m_syscalls[regs.orig_rax]->m_enter_handle) {
                    m_syscalls[regs.orig_rax]->m_enter_handle(&regs, m_pid);
                }
            }
        }
        /* 其他非监听情况，忽略 */
        else if (WIFSTOPPED(status) && WSTOPSIG(status) != 0x85) {
            WC_LOG_ERROR(logger) << "[Monitor::wait]: An unknown signal received! " 
                                 << elf->get_name() << " pid:  " << m_pid
                                 << " status: " << status << std::endl;
            continue;
        }

        if (ptrace(PTRACE_SYSCALL, m_pid, 0, 0) == -1){
             WC_LOG_ERROR(logger) << "[Monitor::wait] PTRACE_SYSCALL failed int syscall-exit-stop, will exit! "
                                 << "errno: " << strerror(errno) << std::endl;
            break;
        }
        if (waitpid(m_pid, &status, 0) == -1){ 
            WC_LOG_ERROR(logger) << "[Monitor::wait] waitpid failed int syscall-exit-stop, will exit! "
                                 << "errno: " << strerror(errno) << std::endl;
            break;
        }
        /* 被跟踪进程非正常终止 */
        if (WIFSIGNALED(status)) {
            WC_LOG_ERROR(logger) << "[Monitor::wait]: tracee terminated abnormally " 
                                 << elf->get_name() << " pid:  " << m_pid
                                 << " status: " << WTERMSIG(status) << std::endl;
            break;
        }
        /* 被跟踪进程正常终止 */
        else if (WIFEXITED(status)) {
            WC_LOG_ERROR(logger) << "[Monitor::wait]: tracee normally exited " 
                                 << elf->get_name() << " pid:  " << m_pid
                                 << " status: " << WTERMSIG(status) << std::endl;
            break;
        }
        /* SIGTRAP */
        else if (WIFSTOPPED(status) && WSTOPSIG(status) == 0x5) {
            struct user_regs_struct regs;
            if (ptrace(PTRACE_GETREGS, m_pid, 0, &regs) == -1) {
                WC_LOG_ERROR(logger) << "[Monitor::wait]: Get regs failed! erron: "
                                     << strerror(errno) << std::endl;
                continue;
            }
            // 非拦截的SIGTRAP，可能是被跟踪程序主动抛出，忽略
            long bp_address = regs.rip-1;
            if (m_bps.find(bp_address) == m_bps.end()) {
                continue;
            }
            {
                BreakPoint::ptr bp = m_bps[bp_address].lock();
                trigger(bp, &regs);
            }
            if (flag_update_1) {
                update_1(config, elf);
                flag_update_1 = false;
            }
            if (flag_update_2) {
                update_2(config, elf);
                flag_update_2 = false;
            }
            continue;
        }
        /* syscall-exit-stop */
        else if (WIFSTOPPED(status) && WSTOPSIG(status) == 0x85) {
            struct user_regs_struct regs;
            if (ptrace(PTRACE_GETREGS, m_pid, 0, &regs) == -1) {
                WC_LOG_ERROR(logger) << "[Monitor::wait]: Get regs failed! erron: "
                                     << strerror(errno) << std::endl;
                continue;
            }
            if (m_syscalls.find(regs.orig_rax) != m_syscalls.end()) {
                if (m_syscalls[regs.orig_rax]->m_exit_handle) {
                    m_syscalls[regs.orig_rax]->m_exit_handle(&regs, m_pid);
                }
            }
        }
        else if (WIFSTOPPED(status) && WSTOPSIG(status) != 0x85) {
            WC_LOG_ERROR(logger) << "[Monitor::wait]: An unknown signal received! " 
                                 << elf->get_name() << " pid:  " << m_pid
                                 << " status: " << status << std::endl;
            continue;
        }
    }
}

void Monitor::initial_funcs(Json::Value& values, TraceeElf::ptr elf) {
    if (!values.isMember(INTERCEPT_FUNC)) {
        WC_LOG_ERROR(logger) << "Config file not found functions information!" << std::endl;
        return;
    }

    for (int i=0; i<values[INTERCEPT_FUNC].size(); ++i) {
        Json::Value& tmp = values[INTERCEPT_FUNC][i];
        if (!tmp.isMember(FUNC_NAME)) {
            continue;
        }
        std::string func_name   = tmp[FUNC_NAME].asString();
        Function::ptr func      = get_func(elf, func_name);
        if (!func) continue;
        initial_functioin(tmp, func);

        m_funcs.insert({func_name, func});
    }
}

void Monitor::initial_functioin(Json::Value& values, Function::ptr func) {
        if (values.isMember(FREQUENCY)) {
            int frequency = values[FREQUENCY].asInt();
            func->m_frequency = frequency > 1 ? frequency : 1;
        }
        if (values.isMember(BACKEND_ENTER)) {
            func->m_enter_name = values[BACKEND_ENTER].asString();
        }
        if (values.isMember(BACKEND_EXIT)) {
            func->m_exit_name = values[BACKEND_EXIT].asString();
        }
        if (values.isMember(LD_PRELOAD)) {
            func->is_preload = true;
        }

        backend_handle enter_handle = get_backend(func->m_enter_name);
        backend_handle exit_handle  = get_backend(func->m_exit_name);
        func->m_enter_handle    = enter_handle;
        func->m_exit_handle     = exit_handle;
}

void Monitor::reinitial_function(Json::Value& values, Function::ptr func) {
    if (values.isMember(FREQUENCY)) {
        if (values[FREQUENCY].asInt() != func->m_frequency) {
            int frequency = values[FREQUENCY].asInt();
            func->m_frequency = frequency > 1 ? frequency : 1;
        }
    }
    else {
        func->m_frequency = 1;
    }

    if (values.isMember(BACKEND_ENTER)) {
        if (values[BACKEND_ENTER].asString() != func->m_enter_name) {
            std::string enter_name = values[BACKEND_ENTER].asString();
            backend_handle enter_handle = get_backend(enter_name);
            func->m_enter_name = enter_name;
            func->m_enter_handle = enter_handle;
        }
    }
    else {
        func->m_enter_name = "";
        func->m_enter_handle = nullptr;
    }

    if (values.isMember(BACKEND_EXIT) && values[BACKEND_EXIT].asString() != func->m_exit_name) {
        if (values[BACKEND_EXIT].asString() != func->m_exit_name) {
            std::string exit_name = values[BACKEND_EXIT].asString();
            backend_handle exit_handle = get_backend(exit_name);
            func->m_exit_handle = exit_handle;
            func->m_exit_name = exit_name;
        }
    }
    else {
        func->m_exit_name = "";
        func->m_exit_handle = nullptr;
    }
}

void Monitor::update_funcs(Json::Value& values, TraceeElf::ptr elf) {
    if (!values.isMember(INTERCEPT_SYSCALL)) {
        WC_LOG_ERROR(logger) << "[Monitor::update_funcs] No function intercept target!"
                             << " Please confirm intercept config path, this modify is invalid!" 
                             << std::endl;
        return;
    }
    func_table  update_funcs;
    for (int i=0; i<values[INTERCEPT_FUNC].size(); ++i) {
        Json::Value& tmp = values[INTERCEPT_FUNC][i];
        if (!tmp.isMember(FUNC_NAME)) continue;
        std::string func_name = tmp[FUNC_NAME].asString();

        // 如果拦截目标已存在，则更新其配置选项
        if (m_funcs.find(func_name) != m_funcs.end()) {
            Function::ptr func = m_funcs[func_name];
            reinitial_function(tmp, func);
            update_breakpoint_exist(func);
            update_funcs.insert({func_name, func});
        }
        // 如果拦截目标不存在，则创建新的Function
        else {
            Function::ptr func = get_func(elf, func_name);
            if (!func) continue;
            initial_functioin(tmp, func);
            update_breakpoint_noexist(func);
            update_funcs.insert({func_name, func});
        }
    }
    m_funcs.swap(update_funcs);

    // 更新断点表
    std::unordered_map<uint64_t, std::weak_ptr<BreakPoint> >    update_bps;
    for (auto iter=m_bps.begin(); iter!=m_bps.end(); ++iter) {
        if (iter->second.lock()) {
            update_bps.insert({iter->first, iter->second});
        }
    }
    m_bps.swap(update_bps);
}

void Monitor::update_breakpoint_noexist(Function::ptr func) {
    // 如果是动态库符号的话，需要在进程入口处更新函数的重定位地址
    if (func->m_flag & FUNC_DELAY || func->m_flag & FUNC_DYN) {
        long new_address = ptrace(PTRACE_PEEKDATA, m_pid, func->m_readdress, NULL);
        if (new_address == -1) {
            WC_LOG_ERROR(logger) << "[Monitor::trigger_entry]: Can't relocation dynamic function: "
                                    << func->m_name << " relocation address: " << func->m_readdress
                                    << std::endl;
            return;
        }
        func->m_address = new_address;
    }
    func->is_already = true;
    uint8_t flag = BP_ENTER;
    if (func->m_flag & FUNC_DELAY) {
        flag |= BP_UPDATE_DELAY;
        func->is_already = false;
    }
    // 只有设置了后端处理逻辑才会开启拦截
    if (func->m_enter_handle || func->m_exit_handle) {
        BreakPoint::ptr bp_func(new BreakPoint(
            flag,
            func->m_address,
            m_pid,
            func->m_enter_handle,
            func
        ));
        func->m_bp_enter = bp_func;
        // 设置拦截返回地址断点标志
        if (func->m_exit_handle) {
            func->m_bp_enter->m_has_exit = true;
        }
        m_bps.insert({func->m_address, bp_func});
        bp_func->enable();
    }
}

void Monitor::update_breakpoint_exist(Function::ptr func) {
    // 更新后的函数不需要进行拦截，则取消其设置断点，如果有的话
    if (!func->m_enter_handle && !func->m_exit_handle) {
        func->m_bp_enter    = nullptr;
        func->m_bp_exit     = nullptr;
    }
    // 更新后的函数需要进行拦截
    else {
        // 不存在首地址断点，则新建
        if (!func->m_bp_enter) {
            uint8_t flag = BP_ENTER;
            if (!func->is_already && func->m_flag & FUNC_DELAY) {
                flag |= BP_UPDATE_DELAY;
            }
            BreakPoint::ptr bp_enter(new BreakPoint{
                flag,
                func->m_address,
                m_pid,
                func->m_enter_handle,
                func
            });
            func->m_bp_enter = bp_enter;
            if (func->m_exit_handle) {
                func->m_bp_enter->m_has_exit = true;
            }
            m_bps.insert({func->m_address, func->m_bp_enter});
            func->m_bp_enter->enable();
        }
        // 已存在首地址断点，则更新
        else {
            func->m_bp_enter->m_handle = func->m_enter_handle;
            // 需要拦截返回地址断点
            if (func->m_exit_handle) {
                func->m_bp_enter->m_has_exit = true;
                if (func->m_bp_exit) {
                    func->m_bp_exit->m_handle = func->m_exit_handle;
                }
            }
            // 不再拦截返回地址断点
            else {
                func->m_bp_enter->m_has_exit = false;
                func->m_bp_exit = nullptr;
            }
        }
    }
}

void Monitor::initial_syscalls(Json::Value& values) {
    if (!values.isMember(INTERCEPT_SYSCALL)) {
        WC_LOG_ERROR(logger) << "[Monitor::initial_syscalls] Config file not found syscalls information!" << std::endl;
        return;
    }
    for (int i=0; i<values[INTERCEPT_SYSCALL].size(); ++i) {
        Json::Value& tmp = values[INTERCEPT_SYSCALL][i];
        if (!tmp.isMember(SYSCALL_NAME)) {
            continue;
        }

        int id = tmp[SYSCALL_NAME].asInt();
        if (id < 0) {
            WC_LOG_ERROR(logger) << "[Monitor::initial_syscalls] Error syscall number " << id << std::endl;
            continue;;
        }
        Syscall::ptr syscall(new Syscall(id));
        syscall->m_id = id;
        if (tmp.isMember(FREQUENCY)) {
            syscall->m_frequency = tmp[FREQUENCY].asInt() > 1 ? tmp[FREQUENCY].asInt() : 1;
        }
        if (tmp.isMember(BACKEND_ENTER)) {
            syscall->m_enter_name = tmp[BACKEND_ENTER].asString();
        }
        if (tmp.isMember(BACKEND_EXIT)) {
            syscall->m_exit_name = tmp[BACKEND_EXIT].asString();
        }

        backend_handle enter_handle = get_backend(syscall->m_enter_name);
        backend_handle exit_handle  = get_backend(syscall->m_exit_name);
        syscall->m_enter_handle     = enter_handle;
        syscall->m_exit_handle      = exit_handle;
        m_syscalls.insert({id, syscall});
    }
}

void Monitor::update_syscalls(Json::Value& values) {
    if (!values.isMember(INTERCEPT_SYSCALL)) {
        WC_LOG_ERROR(logger) << "[Monitor::update_syscalls] No syscall intercept target!"
                             << " Please confirm intercept config path, this modify is invalid!" 
                             << std::endl;
        return;
    }
    std::unordered_map<pid_t, Syscall::ptr> update_syscalls;
    for (int i=0; i<values[INTERCEPT_SYSCALL].size(); ++i) {
        Json::Value& tmp = values[INTERCEPT_SYSCALL][i];
        if (!tmp.isMember(SYSCALL_NAME)) continue;

        int id = tmp[SYSCALL_NAME].asInt();
        if (id < 0) {
            WC_LOG_ERROR(logger) << "[Monitor::update_syscalls] Error syscall number " << id << std::endl;
            continue;
        }
        // 如果拦截目标已存在，则更新配置选项
        if (m_syscalls.find(id) != m_syscalls.end()) {
            if (tmp.isMember(FREQUENCY)) {
                int frequency = tmp[FREQUENCY].asInt() > 1 ? tmp[FREQUENCY].asInt() : 1;
                if (frequency != m_syscalls[id]->m_frequency) {
                    m_syscalls[id]->m_frequency = frequency;
                }
            }
            else {
                m_syscalls[id]->m_frequency = 1;
            }
            if (tmp.isMember(BACKEND_ENTER)) {
                std::string enter_name = tmp[BACKEND_ENTER].asString();
                if (enter_name != m_syscalls[id]->m_enter_name) {
                    backend_handle enter_handle     = get_backend(enter_name);
                    m_syscalls[id]->m_enter_handle  = enter_handle;
                    m_syscalls[id]->m_enter_name    = enter_name;
                }
            }
            else {
                m_syscalls[id]->m_enter_handle  = nullptr;
                m_syscalls[id]->m_enter_name    = "";
            }
            if (tmp.isMember(BACKEND_EXIT)) {
                std::string exit_name = tmp[BACKEND_EXIT].asString();
                if (exit_name != m_syscalls[id]->m_exit_name) {
                    backend_handle exit_handle      = get_backend(exit_name);
                    m_syscalls[id]->m_exit_handle   = exit_handle;
                    m_syscalls[id]->m_exit_name     = exit_name;
                }
            }
            else {
                m_syscalls[id]->m_exit_handle   = nullptr;
                m_syscalls[id]->m_exit_name     = "";
            }
            update_syscalls.insert({id, m_syscalls[id]});
        }
        // 拦截目标不存在，则创建新的Syscall
        else {
            Syscall::ptr syscall(new Syscall(id));
            if (tmp.isMember(FREQUENCY)) {
                syscall->m_frequency = tmp[FREQUENCY].asInt() > 1 ? tmp[FREQUENCY].asInt() : 1;
            }
            if (tmp.isMember(BACKEND_ENTER)) {
                syscall->m_enter_name = tmp[BACKEND_ENTER].asString();
            }
            if (tmp.isMember(BACKEND_EXIT)) {
                syscall->m_exit_name = tmp[BACKEND_EXIT].asString();
            }

            backend_handle enter_handle     = get_backend(syscall->m_enter_name);
            backend_handle exit_handle      = get_backend(syscall->m_exit_name);
            syscall->m_enter_handle         = enter_handle;
            syscall->m_exit_handle          = exit_handle;
            update_syscalls.insert({id, syscall});
        }
    }
    m_syscalls.swap(update_syscalls);
    update_syscalls.clear();
}

void Monitor::get_load_address(pid_t pid) {
    /* 打开/proc/pid/maps文件 */
    char proc_file[64] = "";
    sprintf(proc_file, "/proc/%d/maps", pid);
    FILE* proc_fp = fopen(proc_file, "r");
    if (proc_file == NULL) {
        WC_LOG_ERROR(logger) << "[get_load_address]: Can't open proc file " << proc_file << std::endl;
        m_load_address = 0x0;
        return;
    }

    char line[256] = "";
    char load_address_str[40] = "";
    char *offest_address = NULL;
    long load_address_int = 0;
    fgets(line, 256, proc_fp);

    /* 从proc文件中获取进程加载地址 */
    if ((offest_address = strchr(line, '-')) == NULL) {
        WC_LOG_ERROR(logger) << "[get_load_address]: Can't match load address " << proc_file << std::endl;
        m_load_address = 0x0;
        return;
    }
    strncpy(load_address_str, line, offest_address-line);   
    sscanf(load_address_str, "%lx", &load_address_int); // Converts a string to hex
    
    m_load_address = (uint64_t)load_address_int;
}

void Monitor::trigger(BreakPoint::ptr bp, struct user_regs_struct* regs) {
    if (bp->is_entry()) {
        trigger_entry(bp, regs);
        return;
    }
    if (bp->is_enter()) {
        trigger_enter(bp, regs);
        return;
    }
    if (bp->is_exit()) {
        trigger_exit(bp, regs);
        return;
    }
}

void Monitor::step_over(BreakPoint::ptr bp, struct user_regs_struct* regs) {
    /* 重置IR寄存器, 因为遇到断点导致IR寄存器加一 */
    regs->rip = regs->rip-1;
    if (ptrace(PTRACE_SETREGS, m_pid, 0, regs) == -1) {
        WC_LOG_ERROR(logger) << "[Monitor::step_over] Can't reset IR regster: " << bp->m_address << std::endl;
        exit(-1);
    }

    /* 单步越过断点 */
    bp->disable();
    if (ptrace(PTRACE_SINGLESTEP, m_pid, 0, 0) == -1) {
        WC_LOG_ERROR(logger) << "[Monitor::step_over] Can't single step over: " << bp->m_address << std::endl;
        exit(-1);
    }
    int status;
    waitpid(m_pid, &status, 0);
}

void Monitor::trigger_entry(BreakPoint::ptr bp, struct user_regs_struct* regs) {
    if (!m_funcs.empty()) {
        traverse_insert_breakpoint(m_funcs);
    }

    step_over(bp, regs);
    m_bps.erase(bp->m_address);
    m_bp_entry = nullptr;
}

void Monitor::traverse_insert_breakpoint(func_table& funcs) {
    for (auto iter=funcs.begin(); iter!=funcs.end(); ++iter) {
        // 如果是动态库符号的话，需要在进程入口处更新函数的重定位地址
        if (iter->second->m_flag & FUNC_DELAY || iter->second->m_flag & FUNC_DYN) {
            long new_address = ptrace(PTRACE_PEEKDATA, m_pid, iter->second->m_readdress, NULL);
            if (new_address == -1) {
                WC_LOG_ERROR(logger) << "[Monitor::trigger_entry]: Can't relocation dynamic function: "
                                     << iter->first << " relocation address: " << iter->second->m_readdress
                                     << std::endl;
                continue;
            }
            iter->second->m_address = new_address;
        }
        iter->second->is_already = true;
        uint8_t flag = BP_ENTER;
        if (iter->second->m_flag & FUNC_DELAY) {
            flag |= BP_UPDATE_DELAY;
            iter->second->is_already = false;
        }
        // 只有设置了后端处理逻辑才会开启拦截
        if (iter->second->m_enter_handle || iter->second->m_exit_handle) {
            BreakPoint::ptr bp_func(new BreakPoint(
                flag,
                iter->second->m_address,
                m_pid,
                iter->second->m_enter_handle,
                iter->second
            ));
            iter->second->m_bp_enter = bp_func;
            // 设置拦截返回地址断点标志
            if (iter->second->m_exit_handle) {
                iter->second->m_bp_enter->m_has_exit = true;
            }
            m_bps.insert({iter->second->m_address, bp_func});
            bp_func->enable();
        }
    }
}
 
void Monitor::trigger_enter(BreakPoint::ptr bp, struct user_regs_struct* regs) {
    /*
     * 如果需要拦截函数返回，那么需要在函数入口处设置断点，
     * 因为函数入口地址固定，而函数返回地址一般不固定，如果
     * 该函数还是延迟绑定函数的话，则在函数第一次调用结束后，
     * 还需要在函数返回断点处进行重定位
    */
    if (bp->m_has_exit) {
        Function::ptr func = bp->m_func.lock();
        if (!func) {
            WC_LOG_ERROR(logger) << "[Monitor::trigger_ente] error breakpoint (" << bp->m_address
                                 << ") related with funcion" <<std::endl;
            step_over(bp, regs);
            return;
        }
        uint8_t flag = func->is_already ? BP_EXIT : BP_EXIT|BP_UPDATE_DELAY;
        long ret_address = ptrace(PTRACE_PEEKDATA, m_pid, regs->rsp, NULL);
        if (ret_address != -1) {
            // 复用返回地址断点结构，只需要重置断点地址
            if (!func->m_bp_exit) {
                BreakPoint::ptr return_bp(new BreakPoint(
                    flag,
                    ret_address,
                    m_pid,
                    func->m_exit_handle,
                    func
                ));
                func->m_bp_exit = return_bp;
            }
            func->m_bp_exit->m_flag = flag;
            func->m_bp_exit->m_address = ret_address;

            m_bps.insert({ret_address, func->m_bp_exit});
            func->m_bp_exit->enable();
            WC_LOG_DEBUG(logger) << "trigger enter: (" << func->m_name << "), now breakpoints number: (" << m_bps.size()
                                 << ") I set a exit breakpoint in (" << std::hex << ret_address << std::endl;
        } 
        else {
            WC_LOG_ERROR(logger) << "[Monitor::trigger_enter] Can't peek function " 
                                 << "return address, errno: " 
                                 << strerror(errno) << std::endl;
        }
    }
   
    // 越过断点并重新开启断点
    step_over(bp, regs);

    // 执行后端处理逻辑，如果后端处理逻辑报错，那么将会拦截到异常，并不再拦截该函数
    if (bp->m_handle) {
        try {
            bp->m_handle(regs, m_pid);
        } catch (std::exception& e) {
            Function::ptr func = bp->m_func.lock();
            WC_LOG_ERROR(logger) << "[Monitor::trigger_enter] Your custom function enter handle throw a exception!" 
                                 << " I will not intercept this function (" << func->m_name
                                 << ")" << std::endl;
            return;
        }
    }
    bp->enable();
}
 
void Monitor::trigger_exit(BreakPoint::ptr bp, struct user_regs_struct* regs) {
    WC_LOG_DEBUG(logger) << "[Monitor::trigger_exit] trigger exit handle! function name is: (" << std::endl;

    Function::ptr func = bp->m_func.lock();
    if (!func) {
        WC_LOG_ERROR(logger) << "[Monitor::trigger_ente] error breakpoint (" << bp->m_address
                                << ") related with funcion" <<std::endl;
        step_over(bp, regs);
        return;
    }
    /*
     * 延迟绑定的函数第一次调用时，需要在返回地址断点处更新函数的重定向地址和对应的断点
     * 1. 根据重定位地址从.got中获取新的重定位地址
     * 2. 从断点表中删除原有旧函数入口断点，因为断点表的键是旧函数入口地址
     * 3. 禁止中断
     * 4. 更新原断点地址和函数入口地址，设置already标志，表示该延迟函数已完成更新
     * 5. 开中断
     * 6. 将更新后的断点插入断点表
    */
    if (bp->is_delay()) {
        long new_address = ptrace(PTRACE_PEEKDATA, m_pid, func->m_readdress, NULL);
        if (new_address == -1) {
            WC_LOG_ERROR(logger) << "[Monitor::trigger_entry]: Can't relocation delay dynamic function: "
                                << func->m_name << " relocation address: " 
                                << func->m_readdress << std::endl;
        }
        m_bps.erase(func->m_address);
        func->m_bp_enter->disable();
        
        func->m_bp_enter->m_address = new_address;
        func->m_address             = new_address;
        func->is_already            = true;

        func->m_bp_enter->enable();

        m_bps.insert({new_address, func->m_bp_enter});
    }
    step_over(bp, regs);

    if (bp->m_handle) {
        try {
            bp->m_handle(regs, m_pid);
        } catch (std::exception& e) {
            WC_LOG_ERROR(logger) << "[Monitor::trigger_enter] Your custom function enter handle throw a exception!" 
                                 << " I will not intercept this function (" << func->m_name
                                 << ")" << std::endl;
        }
    }
    func->m_bp_exit->m_address = 0x0;
    m_bps.erase(bp->m_address);
}

void Monitor::update_1(Config::ptr config, TraceeElf::ptr elf) {
    // 更新配置文件
    if (m_config_path.empty() || !config->update()) {
        return;
    }

    // 更新系统调用拦截目标
    update_syscalls(config->m_values);

    // 更新函数拦截目标
    update_funcs(config->m_values, elf);
}

void Monitor::update_2(Config::ptr config, TraceeElf::ptr elf) {
    if (m_config_path.empty() || !config->update()) {
        WC_LOG_ERROR(logger) << "[Monitor::update_2] update config failed!" << std::endl;
        return;
    }
    std::string new_library_path = config->get_string(CONFIG_LIBRARY);
    void *new_library = dlopen(new_library_path.c_str(), RTLD_LAZY);
    if (new_library != NULL) {
        dlclose(m_library);
        m_library = new_library;
    }
    else {
        WC_LOG_ERROR(logger) << "[Monitor::update_2] open new backend library failed!" << std::endl;
        return;
    }
    // 更新系统调用拦截目标
    update_syscalls(config->m_values);

    for (auto iter=m_funcs.begin(); iter!=m_funcs.end(); ++iter) {
        iter->second->m_enter_name      = "";
        iter->second->m_enter_handle    = nullptr;
        iter->second->m_exit_name       = "";
        iter->second->m_exit_handle     = nullptr;
        iter->second->m_frequency       = 1;
    }
    // 更新函数拦截目标
    update_funcs(config->m_values, elf);
}

void Monitor::wait_no_syscall(Config::ptr config, TraceeElf::ptr elf, bool& flag_update_1, bool& flag_update_2) {
    ptrace(PTRACE_CONT, m_pid, 0, 0);
    int status;
    while (waitpid(m_pid, &status, 0)) {
        /* 被跟踪进程非正常终止 */
        if (WIFSIGNALED(status)) {
            WC_LOG_ERROR(logger) << "[Monitor::wait]: tracee terminated abnormally " 
                                 << elf->get_name() << " pid:  " << m_pid
                                 << " status: " << WTERMSIG(status) << std::endl;
            break;
        }
        /* 被跟踪进程正常终止 */
        else if (WIFEXITED(status)) {
            WC_LOG_ERROR(logger) << "[Monitor::wait]: tracee normally exited " 
                                 << elf->get_name() << " pid:  " << m_pid
                                 << " status: " << WTERMSIG(status) << std::endl;
            break;
        }
        /* SIGTRAP */
        else if (WIFSTOPPED(status) && WSTOPSIG(status) == 0x5) {
            struct user_regs_struct regs;
            if (ptrace(PTRACE_GETREGS, m_pid, 0, &regs) == -1) {
                WC_LOG_ERROR(logger) << "[Monitor::wait]: Get regs failed! erron: "
                                     << strerror(errno) << std::endl;
                break;
            }
            // 非拦截的SIGTRAP，可能是被跟踪程序主动抛出，忽略
            long bp_address = regs.rip-1;
            if (m_bps.find(bp_address) == m_bps.end()) {
                WC_LOG_ERROR(logger) << "what happen? " << bp_address << std::endl;
                break;
            }
            WC_LOG_DEBUG(logger) << "breakpoint trigger, breakpoint address: (" << bp_address
                                 << ") now breakpoint number: " << m_bps.size() << std::endl; 

            {
                BreakPoint::ptr bp = m_bps[bp_address].lock();
                trigger(bp, &regs);
            }
            if (flag_update_1) {
                update_1(config, elf);
                flag_update_1 = false;
            }
            if (flag_update_2) {
                update_2(config, elf);
                flag_update_2 = false;
            }
        }
        ptrace(PTRACE_CONT, m_pid, 0, 0);
    }
}

} // namespace wc
