/*
 * @Author: huomax
 * @Date: 2023-05-21 03:31:46
 * @LastEditors: huomax
 * @LastEditTime: 2023-05-28 00:03:31
 * @FilePath: /wgk/wc-intercept/ptrace/src/tracer.c
 * @Description: ptrace拦截工具主程序
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */


#include "wc_include.h"
#include "wc_elf.h"
#include "wc_backend.h"


int main(int argc, char **argv)
{
    if (argc < 2)
        FATAL("too few arguments: %d", argc);

    pid_t pid = fork();
    int status;
    switch (pid) {
        case -1:
            FATAL("%s", strerror(errno));
        case 0: 
            ptrace(PTRACE_TRACEME, 0, 0, 0);
            execvp(argv[2], argv + 2);
            FATAL("%s", strerror(errno));
    }


    waitpid(pid, &status, 0);
    ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_EXITKILL);

    /* 获取指定符号地址 */
    long start_intercept_point = GetSymbolAddress(argv[2], argv[1]) + GetLoadAddress(pid);
 
    /* 单步执行直到指定符号地址 */
    long ret_address = RunTo(pid, status, start_intercept_point);

    /* 开始拦截系统调用 */
    for (;;) {
        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1)
            FATAL("%s", strerror(errno));
        if (wait(&status) == -1) 
            FATAL("%s", strerror(errno));

        /* 执行系统调用前的后端处理逻辑 */
        intercept_backend_enter(pid);
                
        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1)
            FATAL("%s", strerror(errno));
        if (wait(&status) == -1)
            FATAL("%s", strerror(errno));
        
        /* 执行完毕后的系统调用的后端处理逻辑 */
        intercept_backend_exit(pid);
    }
}
