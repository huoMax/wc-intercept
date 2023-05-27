/*
 * @Author: huomax
 * @Date: 2023-05-25 00:39:02
 * @LastEditors: huomax
 * @LastEditTime: 2023-05-25 00:40:04
 * @FilePath: /wgk/wc-intercept/ptrace/sample/trace_include.h
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#ifndef __TRACE_INCLUDE__
#define __TRACE_INCLUDE__

/* C standard library */
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

/* POSIX */
#include <unistd.h>
#include <sys/user.h>
#include <sys/wait.h>

/* Linux */
#include <syscall.h>
#include <sys/ptrace.h>

#define FATAL(...) \
    do { \
        fprintf(stderr, "wc-itercept: " __VA_ARGS__); \
        fputc('\n', stderr); \
        exit(EXIT_FAILURE); \
    } while (0)

#endif 