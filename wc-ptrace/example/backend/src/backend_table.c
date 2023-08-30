/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-08-06 17:06:43
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-18 16:56:14
 * @FilePath: /wc-intercept/wc-ptrace/example/backend/src/backend_table.c
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <stdio.h>
#include <sys/user.h>
#include <sys/ptrace.h>

void blank(struct user_regs_struct* regs, int pid) {
    
}

void printf_rand_enter(struct user_regs_struct* regs, int pid) {
    printf("rand enter\n");
}

void printf_rand_exit(struct user_regs_struct* regs, int pid) {
    printf("rand exit\n");
}

void random_rand_exit(struct user_regs_struct* regs, int pid) {
    regs->rax = 99;
    ptrace(PTRACE_SETREGS, pid, 0, regs);
}

void printf_sleep_enter(struct user_regs_struct* regs, int pid) {
    printf("sleep enter\n");
}

void printf_sleep_exit(struct user_regs_struct* regs, int pid) {
    printf("sleep exit\n");
}