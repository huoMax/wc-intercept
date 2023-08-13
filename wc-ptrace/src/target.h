/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-08-03 17:56:02
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-13 13:01:41
 * @FilePath: /wc-intercept/wc-ptrace/src/target.h
 * @Description: 函数抽象封装
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#ifndef __WC_TARGET_H_
#define __WC_TARGET_H_

#include <stdint.h>
#include <string>
#include <elf.h>
#include <gelf.h>
#include <memory>

#define FUNC_NONE   0x0
#define FUNC_NORMAL 0x1
#define FUNC_DYN    0x2
#define FUNC_DELAY  0x4

#define BP_NONE          0x0          // 无类型
#define BP_ENTER         0x1          // 函数入口断点事件类型
#define BP_EXIT          0x2          // 函数返回断点事件类型
#define BP_ENTRY         0x4          // 程序入口断点事件类型
#define BP_UPDATE_DYN    0x8          // 更新共享库符号事件类型
#define BP_UPDATE_DELAY  0x10         // 更新延迟绑定符号事件类型

namespace wc {
    
typedef void (*backend_handle)(struct user_regs_struct*, int);

class Function;

class BreakPoint {
friend class Monitor;
public:
    typedef std::shared_ptr<BreakPoint> ptr;

    /**
     * @brief: 构造函数
     */
    BreakPoint(uint8_t flag, uint64_t address, pid_t pid, backend_handle handle, std::shared_ptr<Function> func)
        : m_flag(flag)
        , m_address(address)
        , m_pid(pid)
        , m_handle(handle)
        , m_func(func)
    {
        m_enable    = false;
        m_content   = 0x0;
        m_has_exit  = false;
    }

    /**
     * @brief: 析构函数
     */    
    ~BreakPoint() {
        if (m_enable) {
            disable();
        }
    }
        
    // 设置断点标志位
    void set_clear() { m_flag = BP_NONE; }
    void set_start() { m_flag |= BP_ENTER; }
    void set_end()   { m_flag |= BP_EXIT; }
    void set_enter() { m_flag |= BP_ENTRY; }
    void set_update_dyn() { m_flag |= BP_UPDATE_DYN; }
    void set_update_delay() { m_flag |= BP_UPDATE_DELAY; }

    // 判断断点属性
    bool is_enter() { return m_flag & BP_ENTER; }
    bool is_exit() { return m_flag & BP_EXIT; }
    bool is_delay() { return m_flag & BP_UPDATE_DELAY; }
    bool is_dyn() { return m_flag & BP_UPDATE_DYN; }
    bool is_entry() { return m_flag & BP_ENTRY; }
    bool is_enable() { return m_enable; }

    // 更新断点地址
    bool update(uint64_t new_address);

    // 开中断
    bool enable();

    // 关中断
    bool disable();

    uint8_t get_flag() { return m_flag; }

private:
    /* 断点属性 */
    bool        m_enable;    // 断点是否开启
    uint8_t     m_flag;      // 断点类型

    /* 断点插入 */
    uint64_t    m_content;  // 断点插入的指令的内容
    uint64_t    m_address;  // 断点插入的地址

    /* 断点归属 */
    std::weak_ptr<Function> m_func;     // 断点归属的函数
    backend_handle          m_handle;   // 断点关联的后端处理逻辑
    bool                    m_has_exit; // 该断点是首地址断点且需要设置返回地址断点

    /* 其他 */
    pid_t m_pid;  // 断点插入的进程PID
};

struct Function {
    typedef std::shared_ptr<Function> ptr;

    /* 函数属性 */
    uint8_t         m_flag;
    uint64_t        m_address;
    uint64_t        m_readdress;
    std::string     m_name;

    /* 拦截控制 */
    bool            is_already;
    bool            is_preload;
    uint8_t         m_frequency;

    /* 拦截后端 */
    std::string     m_enter_name;
    std::string     m_exit_name;
    backend_handle  m_enter_handle;
    backend_handle  m_exit_handle;
    std::shared_ptr<BreakPoint> m_bp_enter;
    std::shared_ptr<BreakPoint> m_bp_exit;

    Function(const std::string& name, uint8_t flag, uint64_t address)
            : m_name(name)
            , m_flag(flag)
            , m_address(address)
    {
        is_already      = false;
        is_preload      = false;
        m_readdress     = 0x0;
        m_frequency     = 1;

        m_enter_handle  = nullptr;
        m_exit_handle   = nullptr;
        m_exit_name     = "";
        m_enter_name    = "";
        m_bp_enter      = nullptr;
        m_bp_exit       = nullptr;
    }
};

struct Syscall {
    typedef std::shared_ptr<Syscall> ptr;

    /* 系统调用属性 */
    int m_id;
    int m_frequency;

    /* 后端处理逻辑 */
    std::string     m_enter_name;
    std::string     m_exit_name;
    backend_handle  m_enter_handle;
    backend_handle  m_exit_handle;

    Syscall(int id) : m_id(id) {
        m_frequency = 1;

        m_enter_handle  = nullptr;
        m_exit_handle   = nullptr;
        m_enter_name    = "";
        m_exit_name     = "";
    }
};

} // namespace wc

#endif