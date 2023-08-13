/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-08-06 17:06:43
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-13 13:52:22
 * @FilePath: /wc-intercept/wc-ptrace/library/backend_table.c
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <stdio.h>
#include <sys/user.h>
#include <sys/ptrace.h>

void sys_write_enter(struct user_regs_struct* regs, int pid) {
    if (!regs) {
        printf("nullptr regs struct!\n");
        return;
    }
    fprintf(stderr, "syscall id: %lld parameters: (0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)\n",
        regs->orig_rax,
        (long)regs->rdi, (long)regs->rsi, (long)regs->rdx,
        (long)regs->r10, (long)regs->r8,  (long)regs->r9);
}

void sys_write_exit(struct user_regs_struct* regs, int pid) {
    if (!regs) {
        printf("nullptr regs struct!\n");
        return;
    }
    fprintf(stderr, "syscall id: %lld return: (0x%lx)\n",
        regs->orig_rax, (long)regs->rax);
}

void func_printf_enter(struct user_regs_struct* regs, int pid) {
    if (!regs) {
        printf("nullptr regs struct!\n");
        return;
    }
    fprintf(stderr, "func parameters: (0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)\n",
        (long)regs->rdi, (long)regs->rsi, (long)regs->rdx,
        (long)regs->r10, (long)regs->r8,  (long)regs->r9);
}

void func_printf_exit(struct user_regs_struct* regs, int pid) {
    if (!regs) {
        printf("nullptr regs struct!\n");
        return;
    }
    fprintf(stderr, "func return: (0x%lx)\n",
        (long)regs->rax);
}

void test(struct user_regs_struct* regs, int pid) {
    
}

void test_rand_enter(struct user_regs_struct* regs, int pid) {
    printf("rand enter\n");
}

void test_rand_exit(struct user_regs_struct* regs, int pid) {
    printf("rand exit\n");
}

void test_sleep_enter(struct user_regs_struct* regs, int pid) {
    printf("sleep enter\n");
}

void test_sleep_exit(struct user_regs_struct* regs, int pid) {
    printf("sleep exit\n");
}