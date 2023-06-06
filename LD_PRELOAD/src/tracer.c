/*
 * @Author: huomax
 * @Date: 2023-06-06 18:42:56
 * @LastEditors: huomax
 * @LastEditTime: 2023-06-07 05:29:27
 * @FilePath: /wgk/wc-intercept/LD_PRELOAD/src/tracer.c
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syscall.h>
#include <sys/ptrace.h>
#include <sys/user.h>

#include "wc_backend.h"

int intercept_syscall(int pid);
int parser_config(char* config_path, char* ld_path);


int main(int argc, char **argv) {
    if (argc < 2){
            fprintf(stderr, "too few arguments: %d\n", argc);
            exit(-1);
        }

    char ld_path[256];
    if (parser_config(argv[1], ld_path) == -1) {
        fprintf(stderr, "[parser_config]: can't found ini.config\n");
        return -1;
    }
    char *envp[2] = {ld_path, NULL};
    
    pid_t pid = fork();
    int status;
    switch (pid) {
        case -1:
            fprintf(stderr, "%s\n", strerror(errno));
            exit(-1);
        case 0:
            execve(argv[2], argv + 2, envp);
            fprintf(stderr, "%s\n", strerror(errno));
            exit(-1);
    }

    while(waitpid(pid, &status, 0)) {
        if (WIFSIGNALED(status) || WIFEXITED(status)) {
            fprintf(stderr,"tracee %d terminated %d\n", pid, WTERMSIG(status));
            return 0;
        }
        // printf("status: %x, %x\n", status, WSTOPSIG(status));
        ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_EXITKILL|PTRACE_O_TRACESYSGOOD);

        int err_code = intercept_syscall(pid);
        switch (err_code) {
            case 1: break;
            case 0: return -1;
            default: return -1;
        }
    }

    return 0;
}


int parser_config(char* config_path, char* ld_path) {

    FILE *fp = NULL;
    fp = fopen(config_path, "r");
    if (fp == NULL) {
        fprintf(stderr, "[parser_config]: open config file failure, please check config file!\n");
        return -1;
    }

    char line[256] = "";
    while (fgets(line, 256, fp) != NULL) {
        if (strncmp(line, "[LD_PRELOAD]", 12) == 0){
            memset(line, 0, sizeof(line));
            while (fgets(line, 256, fp) != NULL) {
                if (strncmp(line, "LD_PRELOAD=", 11) == 0) {
                    strcpy(ld_path, line);
                }
                memset(line, 0, sizeof(line));
            }
            break;
        }
        memset(line, 0, sizeof(line));
    }

    fclose(fp);
    return 1;
}

int intercept_syscall(int pid) {

    FILE *fp = fopen("record.txt", "a");
    int status;
    for (;;) {
        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1){
            fprintf(stderr, "[tracer]: PTRACE_SYSCALL failed before the syscall-enter-stop!\n");
            fclose(fp);
            return -1;
        }
        if (waitpid(pid, &status, 0) == -1) {
            fprintf(stderr, "[tracer]: waitpid failed before the syscall-enter-stop!\n");
            fclose(fp);
            return -1;
        }
        if (WIFSIGNALED(status)) {
            fprintf(stderr,"[tracer]: tracee %d terminated abnormally with signal %d\n", pid, WTERMSIG(status));
            fclose(fp);
            return 0;
        }
        else if (WIFEXITED(status)) {
            fprintf(stderr, "[tracer]: tracee pid %d normally exited with status %d!\n", pid, WEXITSTATUS(status));
            fclose(fp);
            return 0;
        }
        else if (WIFSTOPPED(status) && WSTOPSIG(status) == 0x13) {
            // fprintf(fp, "capture a breakpoint in before the syscall-enter-stop!\n\n");
            ptrace(PTRACE_DETACH, pid, 0, 0);
            fclose(fp);
            return 1;
        }
        else if (WIFSTOPPED(status) && WSTOPSIG(status) != 0x85) {
            fprintf(stderr, "[tracer]: tracee has unknown error!\n");
            fclose(fp);
            return -1;
        }

        // 系统调用入口的后端处理逻辑接口
        intercept_backend_enter(pid);

        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1){
            fprintf(stderr, "[tracer]: PTRACE_SYSCALL failed before the syscall-exit-stop!\n");
            fclose(fp);
            return -1;
        }
        if (waitpid(pid, &status, 0) == -1){ 
            fprintf(stderr, "[trcaer]: waitpid failed after the syscall-exit-stop!\n");
            fclose(fp);
            return -1;
        }
        if (WIFSIGNALED(status)) {
            fprintf(stderr,"[tracer]: tracee %d terminated abnormally with signal %d\n", pid, WTERMSIG(status));
            fclose(fp);
            return 0;
        }
        else if (WIFEXITED(status)) {
            fprintf(stderr, "[tracer]: tracee pid %d normally exited with status %d!\n", pid, WEXITSTATUS(status));
            fclose(fp);
            return 0;
        }
        else if (WIFSTOPPED(status) && WSTOPSIG(status) == 0x13) {
            // printf("capture a breakpoint in before the syscall-exit-stop!\n");
            ptrace(PTRACE_DETACH, pid, 0, 0);
            fclose(fp);
            return 1;
        }
        else if (WIFSTOPPED(status) && WSTOPSIG(status) != 0x85) {
            fprintf(stderr, "[tracer]: tracee has unknown error!\n");
            return -1;
        }
        
        // 系统调用入口的后端处理逻辑接口
        intercept_backend_exit(pid);
    }
}
