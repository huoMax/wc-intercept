/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-08-13 15:23:38
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-15 19:58:00
 * @FilePath: /wc-intercept/wc-ptrace/backend/include/wc_ptrace.h
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#ifndef __WC_PTRACE_H_
#define __WC_PTRACE_H_

#include <stdio.h>
#include <sys/user.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <asm/unistd.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


#define PEEKSIZE sizeof(long)

 /**
 * @brief: 从被跟踪进程的内存空间中获取指定长度的数据
 * @param {int} pid 被跟踪进程的PID
 * @param {long} src 目标数据在跟踪进程内存中的首地址
 * @param {int} size 数据长度
 * @param {char*} data 由调用者给出，用于存放获取的目标数据，注意给data分配的内存需要大于size
 * @description: 
 * @return {int} 返回-1表示读取未成功
 */
int PeekData(int pid, long src, int size, char* data) {
    if (size <= 0) {
        fprintf(stderr, "[PeekData]: Peek size must be a positive integer!");
        return -1;
    }

    int index = 0;
    while (size > 0) {
        long peek_data = ptrace(PTRACE_PEEKDATA, pid, (unsigned long)src, NULL);
        if (peek_data == -1) {
            fprintf(stderr, "[PeekLong]: Get data failed! erron: %s", strerror(errno));
            return -1;
        }

        if (size < PEEKSIZE) {
            memcpy(data+index, (char *)&peek_data, size);
            break;
        }
        memcpy(data+index, (char *)&peek_data, PEEKSIZE);
        size -= PEEKSIZE;
        index += PEEKSIZE;
    }
    return 1;
}

/**
 * @brief: 从被跟踪进程的内存空间中获取一个整型
 * @param {int} pid 被跟踪进程的PID
 * @param {long} src 目标数据在跟踪进程内存中的首地址
 * @description: 
 * @return {int} 返回获取的整型
 */
int PeekInt(int pid, long src) {
    long data = ptrace(PTRACE_PEEKDATA, pid, (unsigned long)src, NULL);
    if (data == -1) {
        fprintf(stderr, "[PeekInt]: Get data failed! erron: %s", strerror(errno));
        return -1;
    }
    return *(int *)&data;
}

/**
 * @brief: 从被跟踪进程的内存空间中获取一个长整型
 * @param {int} pid 被跟踪进程的PID
 * @param {long} src 目标数据在跟踪进程内存中的首地址
 * @description: 
 * @return {long} 返回获取的长整型
 */
long PeekLong(int pid, long src) {
    long data = ptrace(PTRACE_PEEKDATA, pid, (unsigned long)src, NULL);
    if (data == -1) {
        fprintf(stderr, "[PeekLong]: Get data failed! erron: %s", strerror(errno));
        return -1;
    }
    return data;
}

/**
 * @brief: 从被跟踪进程的内存空间中获取一个字节
 * @param {int} pid 被跟踪进程的PID
 * @param {long} src 目标数据在跟踪进程内存中的首地址
 * @description: 
 * @return {char} 返回获取的字节
 */
char PeekChar(int pid, long src) {
    long data = ptrace(PTRACE_PEEKDATA, pid, src, NULL);
    if (data == -1) {
        fprintf(stderr, "[PeekChar]: Get data failed! erron: %s", strerror(errno));
        return -1;
    }
    return *(char *)&data;
}

/**
 * @brief: 用户向指定地址写入一个长整型
 * @param {int} pid
 * @param {long} src 目标地址
 * @param {long} val 待写入的值
 */
bool PokeLong(int pid, long src, long val) {
    if (ptrace(PTRACE_POKEDATA, pid, (unsigned long)src, (void*)(&val)) == -1)
        return false;
    return true;
}

/**
 * @brief: 用户向指定地址写入一个整型
 * @param {int} pid
 * @param {long} src 目标地址
 * @param {long} val 待写入的值
 */
bool PokeInt(int pid, long src, int val) {
    if (ptrace(PTRACE_POKEDATA, pid, (unsigned long)src, (void*)(&val)) == -1)
        return false;
    return true;
}

/**
 * @brief: 用户向指定地址写入一个字符
 * @param {int} pid
 * @param {long} src 目标地址
 * @param {long} val 待写入的值
 */
bool PokeChar(int pid, long src, char val) {
    if (ptrace(PTRACE_POKEDATA, pid, (unsigned long)src, (void*)(&val)) == -1)
        return false;
    return true;
}

/**
 * @brief: 获取被跟踪进程常用寄存器
 * @param {int} pid 被跟踪进程PID
 * @param {user_regs_struct*} regs 存放被跟踪寄存器的值
 * @description: 
 * @return {int} 读取失败，返回-1
 */
int GetRegs(int pid, struct user_regs_struct* regs) {
    if (ptrace(PTRACE_GETREGS, pid, 0, regs) == -1) {
        return -1;
    }
    return 0;
}

/**
 * @brief: 设置被跟踪进程的寄存器
 * @param {int} pid 被跟踪进程PID
 * @param {user_regs_struct*} regs 用于设置寄存器的值
 * @description: 
 * @return {int} 设置失败，返回-1
 */
int SetRegs(int pid, struct user_regs_struct* regs) {
    if (ptrace(PTRACE_SETREGS, pid, 0, regs) == -1) {
        return -1;
    }
    return 0;
}

#endif