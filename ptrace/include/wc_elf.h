/*
 * @Author: huomax
 * @Date: 2023-05-24 23:42:00
 * @LastEditors: huomax
 * @LastEditTime: 2023-06-06 03:29:21
 * @FilePath: /wgk/wc-intercept/ptrace/include/wc_elf.h
 * @Description: 解析被跟踪进程的ELF文件
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */

#ifndef __WC_ELF_
#define __WC_ELF_

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
 * @brief: 获取符号在ELF文件中的偏移量
 * @param {char*} file_path
 * @param {char*} target_symbol
 * @param {int*} symbol_offset 用于存放偏移量的变量地址
 * @description: 
 * @return {int} 错误码，为-1时表示出现错误
 */
int get_symbol_offset(char* file_path, char* target_symbol, int* symbol_offset);


/**
 * @brief: 获取被跟踪进程在虚拟内存中的加载地址
 * @param {int} pid 被跟踪进程PID
 * @description: 
 * @return {int}
 */
int get_load_address(int pid, long* load_address);


#endif
