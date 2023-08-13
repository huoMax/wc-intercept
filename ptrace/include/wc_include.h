/*
 * @Author: huomax
 * @Date: 2023-05-25 00:39:02
 * @LastEditors: huomax
 * @LastEditTime: 2023-06-06 03:23:24
 * @FilePath: /wgk/wc-intercept/ptrace/include/wc_include.h
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#ifndef __TRACE_INCLUDE_
#define __TRACE_INCLUDE_

/* C standard library */
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

/* POSIX */
#include <unistd.h>
#include <stdint.h>
#include <sys/user.h>
#include <sys/wait.h>

/* Linux */
#include <syscall.h>
#include <sys/ptrace.h>

/**
 * @brief: 拦截系统调用
 * @param {int} pid tracee进程PID
 * @description: 使用PTRACE_SYSCALL拦截系统调用，直到程序结束或遇到断点
 * @return {int} tracee结束或遇到断点时返回-1, 遇到断点时返回1
 */
int intercept_syscall(int pid);

#endif 