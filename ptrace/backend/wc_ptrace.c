/*
 * @Author: huomax
 * @Date: 2023-05-27 23:02:40
 * @LastEditors: huomax
 * @LastEditTime: 2023-05-27 23:33:30
 * @FilePath: /wgk/wc-intercept/ptrace/backend/wc_ptrace.c
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */

#include "wc_ptrace.h"

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
