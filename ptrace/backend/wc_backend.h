/*
 * @Author: huomax
 * @Date: 2023-05-27 19:26:42
 * @LastEditors: huomax
 * @LastEditTime: 2023-05-27 23:08:22
 * @FilePath: /wgk/wc-intercept/ptrace/backend/wc_backend.h
 * @Description: 后端处理逻辑
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */

#ifndef __WC_BACKEND__
#define __WC_BACKEND__

/* Linux */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syscall.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>


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


 #endif
