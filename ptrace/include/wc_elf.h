/*
 * @Author: huomax
 * @Date: 2023-05-24 23:42:00
 * @LastEditors: huomax
 * @LastEditTime: 2023-05-27 22:58:25
 * @FilePath: /wgk/wc-intercept/ptrace/include/wc_elf.h
 * @Description: 解析被跟踪进程的ELF文件
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */

#ifndef __WC_ELF__
#define __WC_ELF__

#include <libelf.h>
#include <gelf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syscall.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>


/**
 * @brief: 运行程序直到指定函数的第一条指令
 * @param {int} pid 被跟踪进程PID
 * @param {int} status 子进程信号, 由wait(&status)返回
 * @param {long} address 指定地址
 * @description: 
 * @return {long} 函数的返回地址
 */
long  RunTo(int pid, int status, long address);

/**
 * @brief: 获取指定符号在ELF中的偏移量
 * @param {char*} file_path ELF文件路径
 * @param {char*} target_symbol 目标符号
 * @description: 
 * @return {*}
 */
long GetSymbolAddress(char* file_path, char* target_symbol);


/**
 * @brief: 获取进程入口点地址
 * @param {char*} file_path ELF文件路径
 * @description: 
 * @return {*} 
 */
long GetEntryAddress(char* file_path);


/**
 * @brief: 获取被跟踪进程在虚拟内存中的加载地址
 * @param {int} pid 被跟踪进程PID
 * @description: 
 * @return {long int}
 */
long GetLoadAddress(int pid);


#endif
