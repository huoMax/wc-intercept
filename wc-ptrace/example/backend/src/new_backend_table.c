/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-08-13 14:40:38
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-13 14:41:00
 * @FilePath: /wc-intercept/wc-ptrace/library/new_backend_table.c
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <stdio.h>
#include <sys/user.h>
#include <sys/ptrace.h>

void test_rand_enter(struct user_regs_struct* regs, int pid) {
    printf("new rand enter\n");
}

void test_rand_exit(struct user_regs_struct* regs, int pid) {
    printf("new rand exit\n");
}

void test_sleep_enter(struct user_regs_struct* regs, int pid) {
    printf("new sleep enter\n");
}

void test_sleep_exit(struct user_regs_struct* regs, int pid) {
    printf("new sleep exit\n");
}