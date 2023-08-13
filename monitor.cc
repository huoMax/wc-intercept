/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-07-29 21:27:00
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-07-31 17:20:02
 * @FilePath: /wc-intercept/wc-ptrace/src/monitor.cc
 * @Description: 监视器模块实现
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include "monitor.h"
#include "config.h"
#include "elf.h"

namespace wc {

void Monitor::initial(ConfigFunc::ptr config_func, ConfigSyscall::ptr config_sys, ConfigVar::ptr config_var, TraceeElf& elf) {
    // 寻找指定拦截函数
    for (auto iter=config_func->m_name.begin(); iter != config_func->m_name.end(); ++iter) {
        Func::ptr func = elf.get_func(iter->first);
        if (func == nullptr) continue;
        else {
            m_func_tables.insert({iter->first, func});
        }
    }

    // 设置函数的拦截后端逻辑
    std::string library_path = config_var->get_string("backend_table");
    open_backend_library(library_path);
    for (auto iter=m_func_tables.begin(); iter!=m_func_tables.end(); ++iter) {
        backend_handle func_enter = get_backend(config_func->m_backend[iter->first].first);
        backend_handle func_exit = get_backend(config_func->m_backend[iter->first].second);
        if (!func_enter) func_enter = DEFAULT_FUNC_HANDLE;
        if (!func_exit) func_exit = DEFAULT_FUNC_HANDLE;
        m_func_handle_tables.insert({iter->first, {func_enter, func_exit}});
    }

    // 设置系统调用的拦截后端逻辑
    for (auto iter=config_sys->m_name.begin(); iter!=config_sys->m_name.end(); ++iter) {
        backend_handle sys_enter = get_backend(config_sys->m_backend[iter->first].first);
        backend_handle sys_exit = get_backend(config_sys->m_backend[iter->first].second);
        if (!sys_enter) sys_enter = DEFAULT_SYS_HANDLE;
        if (!sys_exit) sys_exit = DEFAULT_SYS_HANDLE;
        m_syscall_handle_tables.insert({iter->first, {sys_enter, sys_exit}});
    }

    syscall_number = m_syscall_handle_tables.size();
    func_number = m_func_handle_tables.size();
}

void Monitor::open_backend_library(const std::string& library_path) {
    if (m_library) {
        dlclose(m_library);
    }
    m_library = dlopen(library_path.c_str(), RTLD_LAZY);
    if (!m_library) {
        std::cout << "Can't open backend library!" << std::endl; 
    }
}

backend_handle Monitor::get_backend(const std::string& backend_name) {
    if (backend_name.empty()) {
        return nullptr;
    }

    backend_handle func = (backend_handle)dlsym(m_library, backend_name.c_str());
    return func;
}

bool Monitor::wait_until_entry(TraceeElf& elf, long load_address, pid_t pid) {
    long entry_address = elf.get_entry()+load_address;
    wc::BreakPoint::ptr bp_entry(new BreakPoint(false, (uint64_t)(entry_address)));
    bp_entry->enable(pid);
    m_breakpoints.insert({bp_entry->m_address, bp_entry});

    int status;
    ptrace(PTRACE_CONT, pid, 0, 0);
    while (waitpid(pid, &status, 0)) {
        if (WIFSIGNALED(status) || WIFEXITED(status)) {
            std::cout << "[Monitor::insert_entry]: tracee already exit!" << std::endl;
            m_breakpoints.erase(bp_entry->m_address);
            return false;
        }
        // 断点
        else if (WIFSTOPPED(status) && WSTOPSIG(status) == 0x5) {
            struct user_regs_struct regs;
            if (ptrace(PTRACE_GETREGS, pid, 0, &regs) == -1) {
                fprintf(stderr, "[Monitor::insert_entry]: Get regs failed! erron: %s\n", strerror(errno));
                m_breakpoints.erase(bp_entry->m_address);
                return false;
            }
            if (regs.rip-1 == bp_entry->m_address) {
                bp_entry->step_over(pid, regs, m_breakpoints);
                update_sym(pid, (uint64_t)load_address);
                break;
            }
            else {
                std::cout << "I don't know what happen!" <<std::endl;
                m_breakpoints.erase(bp_entry->m_address);
                return false;
            }
        }
        else {
            std::cout << "[Monitor::insert_entry]: Unknown error in load process!" << std::endl;
            m_breakpoints.erase(bp_entry->m_address);
            return false;
        }
    }
    m_breakpoints.erase(bp_entry->m_address);
    return true;
}

void Monitor::update_sym(pid_t pid, uint64_t load_address) {
    for (auto iter=m_func_tables.begin(); iter!=m_func_tables.end(); ++iter) {
        iter->second->update_address(pid, load_address);
        if (iter->second->bp_enter) {
            iter->second->bp_enter->enable(pid);
            m_breakpoints.insert({iter->second->bp_enter->m_address, iter->second->bp_enter});
        }
        if (iter->second->bp_exit) {
            iter->second->bp_exit->enable(pid);
            m_breakpoints.insert({iter->second->bp_exit->m_address, iter->second->bp_exit});
        }
    }
}

void Monitor::wait(pid_t pid) {
    int status;
    for (;;) {
        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1){
            fprintf(stderr, "[Monitor::wait]: PTRACE_SYSCALL failed before the syscall-enter-stop!\n");
            break;
        }
        if (waitpid(pid, &status, 0) == -1) {
            fprintf(stderr, "[Monitor::wait]: waitpid failed before the syscall-enter-stop!\n");
            break;
        }
        // 被跟踪程序突然终止
        if (WIFSIGNALED(status)) {
            fprintf(stderr,"[Monitor::wait]: tracee %d terminated abnormally with signal %d\n", pid, WTERMSIG(status));
            break;
        }
        // 被跟踪进程正常终止
        else if (WIFEXITED(status)) {
            fprintf(stderr, "[Monitor::wait]: tracee pid %d normally exited with status %d!\n", pid, WEXITSTATUS(status));
            break;
        }
        // 发生断点
        else if (WIFSTOPPED(status) && WSTOPSIG(status) == 0x5) {
            struct user_regs_struct regs;
            if (ptrace(PTRACE_GETREGS, pid, 0, &regs) == -1) {
                fprintf(stderr, "[Monitor::wait]: Get regs failed! erron: %s\n", strerror(errno));
                break;
            }
            // 检测断点归属
            if (m_breakpoints.find(regs.rip-1) != m_breakpoints.end()) {
                Func::ptr func = m_breakpoints[regs.rip-1]->m_func.lock();
                if (func == nullptr) {
                    fprintf(stderr, "[Monitor::wait]: An unknown breakpoint received, address is: %llx!\n", regs.rip-1);
                    m_breakpoints[regs.rip-1]->step_over(pid, regs, m_breakpoints);
                    continue;
                }
                // 如果是函数入口断点
                if (m_breakpoints[regs.rip-1]->m_is_enter) {
                    m_func_handle_tables[func->m_name].first(&regs);
                }
                else {
                    m_func_handle_tables[func->m_name].second(&regs);
                }
            }
            // 未知的断点
            // fprintf(stderr, "[Monitor::wait]: An unknown breakpoint received, address is: %llx!\n", regs.rip-1);
            continue;
        }
        // 系统调用入口
        else if (WIFSTOPPED(status) && WSTOPSIG(status) == 0x85) {
            struct user_regs_struct regs;
            if (ptrace(PTRACE_GETREGS, pid, 0, &regs) == -1) {
                fprintf(stderr, "[Monitor::wait]: Get regs failed! erron: %s\n", strerror(errno));
                break;
            }
            // 检测系统调用是否是拦截目标
            if (m_syscall_handle_tables.find(regs.orig_rax) != m_syscall_handle_tables.end()) {
                m_syscall_handle_tables[regs.orig_rax].first(&regs);
            }
        }
        // 既非SIGSTOP，也不是sys-enter-stop，忽略
        else if (WIFSTOPPED(status) && WSTOPSIG(status) != 0x85) {
            fprintf(stderr, "[Monitor::wait]: An unknown signal received!\n");
            continue;
        }

        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1){
            fprintf(stderr, "[Monitor::wait]: PTRACE_SYSCALL failed before the syscall-exit-stop!\n");
            break;
        }
        if (waitpid(pid, &status, 0) == -1){ 
            fprintf(stderr, "[Monitor::wait]: waitpid failed after the syscall-exit-stop!\n");
            break;
        }
        if (WIFSIGNALED(status)) {
            fprintf(stderr,"[Monitor::wait]: tracee %d terminated abnormally with signal %d\n", pid, WTERMSIG(status));
            break;
        }
        else if (WIFEXITED(status)) {
            fprintf(stderr, "[Monitor::wait]: tracee pid %d normally exited with status %d!\n", pid, WEXITSTATUS(status));
            break;
        }
        else if (WIFSTOPPED(status) && WSTOPSIG(status) == 0x5) {
            struct user_regs_struct regs;
            if (ptrace(PTRACE_GETREGS, pid, 0, &regs) == -1) {
                fprintf(stderr, "[Monitor::wait]: Get regs failed! erron: %s\n", strerror(errno));
                break;
            }
            // 检测断点归属
            if (m_breakpoints.find(regs.rip-1) != m_breakpoints.end()) {
                Func::ptr func = m_breakpoints[regs.rip-1]->m_func.lock();
                if (func == nullptr) {
                    fprintf(stderr, "[Monitor::wait]: An unknown breakpoint received, address is: %llx!\n", regs.rip-1);
                    m_breakpoints[regs.rip-1]->step_over(pid, regs, m_breakpoints);
                    continue;
                }
                // 如果是函数入口断点
                if (m_breakpoints[regs.rip-1]->m_is_enter) {
                    m_func_handle_tables[func->m_name].first(&regs);
                }
                else {
                    m_func_handle_tables[func->m_name].second(&regs);
                }
            }
            // 未知的断点
            // fprintf(stderr, "[Monitor::wait]: An unknown breakpoint received, address is: %llx!\n", regs.rip-1);
            continue;
        }
        else if (WIFSTOPPED(status) && WSTOPSIG(status) != 0x85) {
            fprintf(stderr, "[Monitor::wait]: An unknown signal received!\n");
            continue;
        }
        // 系统调用结束
        else if (WIFSTOPPED(status) && WSTOPSIG(status) == 0x85) {
            struct user_regs_struct regs;
            if (ptrace(PTRACE_GETREGS, pid, 0, &regs) == -1) {
                fprintf(stderr, "[Monitor::wait]: Get regs failed! erron: %s\n", strerror(errno));
                break;
            }
            if (m_syscall_handle_tables.find(regs.orig_rax) != m_syscall_handle_tables.end()) {
                m_syscall_handle_tables[regs.orig_rax].second(&regs);
            }
        }
    }
}

void Monitor::wait_null(pid_t pid) {
    ptrace(PTRACE_CONT, pid, 0, 0);
}

long DEFAULT_HANDLE(struct user_regs_struct* regs) {
    return 0x0;
}

long DEFAULT_FUNC_HANDLE(struct user_regs_struct* regs) {
    fprintf(stderr, "function(0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)\n",
    (long)regs->rdi, (long)regs->rsi, (long)regs->rdx,
    (long)regs->r10, (long)regs->r8,  (long)regs->r9,(long)regs->rip);
    return 0x0;
}

long DEFAULT_SYS_HANDLE(struct user_regs_struct* regs) {
    fprintf(stderr, "syscall %ld(0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)\n",
    (long)regs->orig_rax,
    (long)regs->rdi, (long)regs->rsi, (long)regs->rdx,
    (long)regs->r10, (long)regs->r8,  (long)regs->r9,(long)regs->rip);
    return 0x0;
}

}