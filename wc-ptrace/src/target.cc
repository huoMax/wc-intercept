/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-08-13 12:55:34
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-13 12:58:43
 * @FilePath: /wc-intercept/wc-ptrace/src/target.cc
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */

#include "target.h"
#include "log.h"

#include <sys/ptrace.h>
#include <string.h>

static wc::Logger::ptr logger = WC_LOG_NAME("intercept");

namespace wc
{
bool BreakPoint::enable() {
    if (m_pid==-1 || m_address == 0x0) {
        WC_LOG_ERROR(logger) << "This is a uninitialize breakpoint, can't enable!" 
                             << std::endl;
        return false;
    }
    long data = ptrace(PTRACE_PEEKDATA, m_pid, m_address, NULL);
    if (data == -1) {
        WC_LOG_ERROR(logger) << "[BreakPoint::enable]: peek instruction (" << m_address << ") content failed!" 
                             << "errno: " << strerror(errno) << std::endl;
        return false;
    }

    uint64_t data_with_int3 = ((data & ~0xff) | 0xcc);
    if (ptrace(PTRACE_POKEDATA, m_pid, m_address, data_with_int3) == -1) {
        WC_LOG_ERROR(logger) << "[BreakPoint::enable]: poke instruction " << m_address << " content failed!" 
                             << "errno: " << strerror(errno) << std::endl;
        return false;
    }
    m_content = (uint8_t)(data & 0xff);
    m_enable = true;
    return true;
}

bool BreakPoint::disable() {
    if (m_pid==-1 || m_address==0x0 || !m_enable) {
        WC_LOG_ERROR(logger) << "This is a uninitialize or enable breakpoint, can't disable!" 
                             << std::endl;
        return false;
    }
    long data = ptrace(PTRACE_PEEKDATA, m_pid, m_address, NULL);
    if (data == -1) {
        WC_LOG_ERROR(logger) << "[BreakPoint::disable]: peek instruction (" << m_address << ") content failed!" 
                             << "errno: " << strerror(errno) << std::endl;
        return false;
    }
    uint64_t restore_data = ((data & ~0xff) | m_content);
    if (ptrace(PTRACE_POKEDATA, m_pid, m_address, restore_data) == -1) {
        WC_LOG_ERROR(logger) << "[BreakPoint::disable]: poke instruction " << m_address << " content failed!" 
                             << "errno: " << strerror(errno) << std::endl;
        return false;
    }
    m_enable = false;
    return true;
}

bool BreakPoint::update(uint64_t new_address) {
    if (m_enable) {
        disable();
        m_address = new_address;
        enable();
    }
    else {
        m_address = new_address;
    }
    return true;
}
} // namespace wc


