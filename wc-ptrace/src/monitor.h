/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-08-03 17:45:38
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-14 21:05:07
 * @FilePath: /wc-intercept/wc-ptrace/src/monitor.h
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#ifndef __WC_MONITOR_H_
#define __WC_MONITOR_H_

#include <dlfcn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <unordered_map>
#include <json/json.h>

#include "config.h"
#include "target.h"
#include "elf.h"
#include "log.h"

#define UPDATE_FLAG_0 0x0
#define UPDATE_FLAG_1 0x1
#define UPDATE_FLAG_2 0x2

namespace wc {

static uint8_t update_flag = UPDATE_FLAG_0;

class Monitor {
public:
    typedef std::shared_ptr<Monitor> ptr;
    typedef std::unordered_map<std::string, Function::ptr> func_table;
    Monitor(Config::ptr config, TraceeElf::ptr elf, pid_t pid);

    ~Monitor();

    void open_backend_library(const std::string& library_path);

    Function::ptr get_func(TraceeElf::ptr elf, const std::string& symbol_name);

    backend_handle get_backend(const std::string& backend_name);

    void set_config_path(const std::string& config_path) { m_config_path = config_path; }

    void trigger(BreakPoint::ptr bp, struct user_regs_struct* regs);

    void trigger_entry(BreakPoint::ptr bp, struct user_regs_struct* regs);

    void trigger_enter(BreakPoint::ptr bp, struct user_regs_struct* regs);

    void trigger_exit(BreakPoint::ptr bp, struct user_regs_struct* regs);

    int wait(Config::ptr config, TraceeElf::ptr elf, bool& flag_update_1, bool& flag_update_2);

    int wait_no_syscall(Config::ptr config, TraceeElf::ptr elf, bool& flag_update_1, bool& flag_update_2);

    void entry(TraceeElf::ptr elf);

    bool is_empty_funcs() {return m_funcs.empty();}

    bool is_empty_syscalls() {return m_bps.empty();}

private:

    void initial_funcs(Json::Value& values, TraceeElf::ptr elf);

    void initial_syscalls(Json::Value& values);

    void update_syscalls(Json::Value& values);

    void update_funcs(Json::Value& values, TraceeElf::ptr elf);

    void update_breakpoint_exist(Function::ptr func);

    void update_breakpoint_noexist(Function::ptr func);

    void get_load_address(pid_t pid);

    void step_over(BreakPoint::ptr bp, struct user_regs_struct* regs);

    void initial_functioin(Json::Value& values, Function::ptr func);

    void reinitial_function(Json::Value& values, Function::ptr func);

    void traverse_insert_breakpoint(func_table& funcs);

    /**
     * @brief: 更新拦截目标和拦截后端处理逻辑，不改变加载的动态库
     */    
    void update_1(Config::ptr config, TraceeElf::ptr elf);

    void update_2(Config::ptr config, TraceeElf::ptr elf);

private:
    void*       m_library;
    bool        m_has_dyns;
    pid_t       m_pid;
    uint64_t    m_load_address;
    std::string m_config_path;
    BreakPoint::ptr m_bp_entry = nullptr;

    std::unordered_map<pid_t, Syscall::ptr >                    m_syscalls;
    std::unordered_map<uint64_t, std::weak_ptr<BreakPoint> >    m_bps;
    std::unordered_map<std::string, Function::ptr>              m_funcs;
};
}

#endif