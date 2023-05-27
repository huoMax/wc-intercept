/*
 * @Author: huomax
 * @Date: 2023-05-27 19:26:50
 * @LastEditors: huomax
 * @LastEditTime: 2023-05-27 23:31:18
 * @FilePath: /wgk/wc-intercept/ptrace/backend/wc_backend.c
 * @Description: 后端处理逻辑
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */

#include "wc_ptrace.h"
#include "wc_backend.h"

void intercept_backend_enter(int pid) {
    struct user_regs_struct regs;
    GetRegs(pid, &regs);
    long syscall = regs.orig_rax;
    fprintf(stderr, "%ld(0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)",
            syscall,
            (long)regs.rdi, (long)regs.rsi, (long)regs.rdx,
            (long)regs.r10, (long)regs.r8,  (long)regs.r9,(long)regs.rip);
}


void intercept_backend_exit(int pid) {
    struct user_regs_struct regs;
    if (GetRegs(pid, &regs) == -1) {
        fputs(" = ?\n", stderr);
        if (errno == ESRCH)
            exit(regs.rdi); // system call was _exit(2) or similar
        fprintf(stderr, "%s", strerror(errno));
    }
    fprintf(stderr, " = %ld\n", (long)regs.rax);
}
