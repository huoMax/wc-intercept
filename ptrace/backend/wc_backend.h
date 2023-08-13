/*
 * @Author: huomax
 * @Date: 2023-05-27 19:26:42
 * @LastEditors: huomax
 * @LastEditTime: 2023-06-06 05:27:10
 * @FilePath: /wgk/wc-intercept/ptrace/backend/wc_backend.h
 * @Description: 后端处理逻辑
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */

#ifndef __WC_BACKEND_
#define __WC_BACKEND_

/* Linux */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syscall.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>

#define PEEKSIZE sizeof(long)

/**
 * @brief: 系统调用入口前的后端处理逻辑
 * @param {int} pid 进程PID
 * @description: 
 * @return {*}
 */
void intercept_backend_enter(int pid);


/**
 * @brief: 系统调用结束后的后端处理逻辑
 * @param {int} pid进程PID
 * @description: 
 * @return {*}
 */
void intercept_backend_exit(int pid);

 /**
 * @brief: 从被跟踪进程的内存空间中获取指定长度的数据
 * @param {int} pid 被跟踪进程的PID
 * @param {long} src 目标数据在跟踪进程内存中的首地址
 * @param {int} size 数据长度
 * @param {char*} data 由调用者给出，用于存放获取的目标数据，注意给data分配的内存需要大于size
 * @description: 
 * @return {int} 返回-1表示读取未成功
 */
int PeekData(int pid, long src, int size, char* data);


/**
 * @brief: 从被跟踪进程的内存空间中获取一个整型
 * @param {int} pid 被跟踪进程的PID
 * @param {long} src 目标数据在跟踪进程内存中的首地址
 * @description: 
 * @return {int} 返回获取的整型
 */
int PeekInt(int pid, long src);

/**
 * @brief: 从被跟踪进程的内存空间中获取一个长整型
 * @param {int} pid 被跟踪进程的PID
 * @param {long} src 目标数据在跟踪进程内存中的首地址
 * @description: 
 * @return {long} 返回获取的长整型
 */
long PeekLong(int pid, long src);


/**
 * @brief: 从被跟踪进程的内存空间中获取一个字节
 * @param {int} pid 被跟踪进程的PID
 * @param {long} src 目标数据在跟踪进程内存中的首地址
 * @description: 
 * @return {char} 返回获取的字节
 */
char PeekChar(int pid, long src);


/**
 * @brief: 获取被跟踪进程常用寄存器
 * @param {int} pid 被跟踪进程PID
 * @param {user_regs_struct*} regs 存放被跟踪寄存器的值
 * @description: 
 * @return {int} 读取失败，返回-1
 */
int GetRegs(int pid, struct user_regs_struct* regs);


/**
 * @brief: 设置被跟踪进程的寄存器
 * @param {int} pid 被跟踪进程PID
 * @param {user_regs_struct*} regs 用于设置寄存器的值
 * @description: 
 * @return {int} 设置失败，返回-1
 */
int SetRegs(int pid, struct user_regs_struct* regs);


 #endif
