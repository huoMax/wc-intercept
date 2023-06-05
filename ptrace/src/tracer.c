/*
 * @Author: huomax
 * @Date: 2023-05-21 03:31:46
 * @LastEditors: huomax
 * @LastEditTime: 2023-06-06 06:06:28
 * @FilePath: /wgk/wc-intercept/ptrace/src/tracer.c
 * @Description: ptrace拦截工具主程序
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */


#include "wc_include.h"
#include "wc_backend.h"
#include "wc_parser.h"


int main(int argc, char **argv)
{
    if (argc < 2){
        fprintf(stderr, "[wc-intercept]: too few arguments: %d\n", argc);
        exit(-1);
    }

    pid_t pid = fork();
    int status;
    switch (pid) {
        case -1:
            fprintf(stderr, "[wc-intercept]: %s\n", strerror(errno));
            exit(-1);
        case 0: 
            ptrace(PTRACE_TRACEME, 0, 0, 0);
            execvp(argv[2], argv + 2);
            fprintf(stderr, "[wc-intercept]: %s\n", strerror(errno));
            exit(-1);
    }

    waitpid(pid, &status, 0);
    ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_EXITKILL|PTRACE_O_TRACESYSGOOD);

    // 初始化断点结构
    int count = 0;
    struct BreakPoint* breakpoint_array = parser_config(argv[1], &count);
    int err_code = 0;

    // 无需设置断点，直接拦截所有系统调用
    if (breakpoint_array == NULL) {
        int err_code = intercept_syscall(pid);
        if(err_code == -1 || err_code == 1) {
            exit(-1);
        }
        else {
            return 0;
        }
    }
    
    // 设置断点
    err_code = breakpoint_initialize(breakpoint_array, count, argv[2], pid);
    if (err_code == -1) {
        exit(-1);
    }
    ptrace(PTRACE_CONT, pid, 0, 0);

    int n = 0;
    while (waitpid(pid, &status, 0)) {
        n++;
        if (n>100) {return -1;}
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            printf("[wc-intercept]: tracee exit with exit code %d\n", WEXITSTATUS(status));
            break;
        }

        // 检测到断点
        if (WIFSTOPPED(status) && WSTOPSIG(status) == 0x5) {

            // 单步越过断点
            step_over_breakpoint(breakpoint_array, count, pid);

            // 拦截系统调用
            int err_code = intercept_syscall(pid);
            if (err_code == -1 || err_code == 0) {
                break;
            }

            // 越过断点
            step_over_breakpoint(breakpoint_array, count, pid);
            ptrace(PTRACE_CONT, pid, 0, 0);
        }
        else {
            ptrace(PTRACE_CONT, pid, 0, 0);
        }
    }

    free(breakpoint_array);
    return 0;
}


int intercept_syscall(int pid) {

    int status;
    for (;;) {
        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1){
            fprintf(stderr, "[intercept_syscall]: PTRACE_SYSCALL failed before the syscall-enter-stop!\n");
            return -1;
        }
        if (waitpid(pid, &status, 0) == -1) {
            fprintf(stderr, "[intercept_syscall]: waitpid failed before the syscall-enter-stop!\n");
            return -1;
        }
        if (WIFSIGNALED(status)) {
            fprintf(stderr,"[intercept_syscall]: tracee %d terminated abnormally with signal %d\n", pid, WTERMSIG(status));
            return 0;
        }
        else if (WIFEXITED(status)) {
            fprintf(stderr, "[intercept_syscall]: tracee pid %d normally exited with status %d!\n", pid, WEXITSTATUS(status));
            return 0;
        }
        else if (WIFSTOPPED(status) && WSTOPSIG(status) == 0x5) {
            // printf("capture a breakpoint in before the syscall-enter-stop!\n");
            return 1;
        }
        else if (WIFSTOPPED(status) && WSTOPSIG(status) != 0x85) {
            fprintf(stderr, "[intercept_syscall]: tracee has unknown error!\n");
            return -1;
        }

        // 系统调用入口的后端处理逻辑接口
        intercept_backend_enter(pid);

        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1){
            fprintf(stderr, "[intercept_syscall]: PTRACE_SYSCALL failed before the syscall-exit-stop!\n");
            return -1;
        }
        if (waitpid(pid, &status, 0) == -1){ 
            fprintf(stderr, "[intercept_syscall]: waitpid failed after the syscall-exit-stop!\n");
            return -1;
        }
        if (WIFSIGNALED(status)) {
            fprintf(stderr,"[intercept_syscall]: tracee %d terminated abnormally with signal %d\n", pid, WTERMSIG(status));
            return 0;
        }
        else if (WIFEXITED(status)) {
            fprintf(stderr, "[intercept_syscall]: tracee pid %d normally exited with status %d!\n", pid, WEXITSTATUS(status));
            return 0;
        }
        else if (WIFSTOPPED(status) && WSTOPSIG(status) == 0x5) {
            // printf("capture a breakpoint in before the syscall-exit-stop!\n");
            return 1;
        }
        else if (WIFSTOPPED(status) && WSTOPSIG(status) != 0x85) {
            fprintf(stderr, "[intercept_syscall]: tracee has unknown error!\n");
            return -1;
        }
        
        // 系统调用入口的后端处理逻辑接口
        intercept_backend_exit(pid);
    }
}
