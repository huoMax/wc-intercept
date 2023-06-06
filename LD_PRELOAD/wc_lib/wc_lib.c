/*
 * @Author: huomax
 * @Date: 2023-06-06 18:45:47
 * @LastEditors: huomax
 * @LastEditTime: 2023-06-07 05:14:22
 * @FilePath: /wgk/wc-intercept/LD_PRELOAD/wc_lib/wc_lib.c
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <syscall.h>
#include <sys/ptrace.h>
#include <signal.h>
#include <unistd.h>

#define LIBC_SYMBOL_ADDR(symbol) dlsym(RTLD_NEXT, #symbol)
#define LIBC_DECLARE(symbol, return_type, ...) return_type symbol(__VA_ARGS__);

int rand() {

    // 请求父进程跟踪
    ptrace(PTRACE_TRACEME, 0, 0, 0);
    raise(SIGSTOP);

    // 要覆盖的动态库函数逻辑
    printf("rand!\n");

    // 请求终止跟踪
    raise(SIGSTOP);
    return 42;
}
