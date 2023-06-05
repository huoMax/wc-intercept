/*
 * @Author: huomax
 * @Date: 2023-05-27 19:26:50
 * @LastEditors: huomax
 * @LastEditTime: 2023-06-06 06:01:17
 * @FilePath: /wgk/wc-intercept/ptrace/backend/wc_backend.c
 * @Description: 后端处理逻辑
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */

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
        fprintf(stderr, "%s", strerror(errno));
    }
    fprintf(stderr, " = %ld\n", (long)regs.rax);
}

 int PeekData(int pid, long src, int size, char* data) {

    if (size <= 0) {
        fprintf(stderr, "[PeekData]: Peek size must be a positive integer!");
        return -1;
    }

    int index = 0;
    while (size > 0) {
        long peek_data = ptrace(PTRACE_PEEKDATA, pid, src, NULL);
        if (peek_data == -1) {
            fprintf(stderr, "[PeekLong]: Get data failed! erron: %s", strerror(errno));
            return -1;
        }

        if (size < PEEKSIZE) {
            memcpy(data+index, (char *)&peek_data, size);
            break;
        }
        memcpy(data+index, (char *)&peek_data, PEEKSIZE);
        size -= PEEKSIZE;
        index += PEEKSIZE;
    }
    return 1;
}

long PeekLong(int pid, long src) {

    long data = ptrace(PTRACE_PEEKDATA, pid, src, NULL);
    if (data == -1) {
        fprintf(stderr, "[PeekLong]: Get data failed! erron: %s", strerror(errno));
        return -1;
    }
    return data;
}

int PeekInt(int pid, long src) {

    long data = ptrace(PTRACE_PEEKDATA, pid, src, NULL);
    if (data == -1) {
        fprintf(stderr, "[PeekInt]: Get data failed! erron: %s", strerror(errno));
        return -1;
    }
    return *(int *)&data;
}

char PeekChar(int pid, long src) {

    long data = ptrace(PTRACE_PEEKDATA, pid, src, NULL);
    if (data == -1) {
        fprintf(stderr, "[PeekChar]: Get data failed! erron: %s", strerror(errno));
        return -1;
    }
    return *(char *)&data;
}

int GetRegs(int pid, struct user_regs_struct* regs) {
    if (ptrace(PTRACE_GETREGS, pid, 0, regs) == -1) {
        return -1;
    }
    return 0;
}

int SetRegs(int pid, struct user_regs_struct* regs) {
    if (ptrace(PTRACE_SETREGS, pid, 0, regs) == -1) {
        return -1;
    }
    return 0;
}
