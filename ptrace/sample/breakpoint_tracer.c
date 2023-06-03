/*
 * @Author: huomax
 * @Date: 2023-06-03 03:51:47
 * @LastEditors: huomax
 * @LastEditTime: 2023-06-04 07:14:58
 * @FilePath: /wgk/wc-intercept/ptrace/sample/breakpoint_tracer.c
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */


#include "wc_include.h"
#include "wc_elf.h"

#define FATAL(...) \
    do { \
        fprintf(stderr, "wc-itercept: " __VA_ARGS__); \
        fputc('\n', stderr); \
        exit(EXIT_FAILURE); \
    } while (0)


struct Record {
    uint64_t address;
    uint8_t save_data;
    uint8_t enable;
};

struct BreakPoint {
    char symbol[64];
    struct Record start;
    struct Record end;
};


/**
 * @brief: 插入断点
 * @param {Record*} record
 * @param {char *} symbol
 * @param {int} pid
 * @description: 替换指定地址指令，插入断点，将被替换指令内容保存在record.save_data中
 * @return {int} 失败则返回false
 */
int breakpoint_enable(struct Record* record, char * symbol, int pid) {

    long data = ptrace(PTRACE_PEEKDATA, pid, record->address, NULL);
    if (data == -1) {
        fprintf(stderr, "[breakpoint_enable]: Get symbol: %s instruction content failed! erron: %s", symbol, strerror(errno));
        return -1;
    }
    record->save_data = (uint8_t)(data & 0xff);

    uint64_t data_with_int3 = ((data & ~0xff) | 0xcc);
    if (ptrace(PTRACE_POKEDATA, pid, record->address, data_with_int3) == -1){
        fprintf(stderr, "[breakpoint_enable]: Insert symbol: %s instruction content failed! erron: %s", symbol, strerror(errno));
        return -1;
    }
    record->enable = 1;
    return 1;
}


/**
 * @brief: 禁止断点
 * @param {Record*} record
 * @param {char*} symbol
 * @param {int} pid
 * @description: 恢复指定地址指令，使用指令原本的内容替换断点
 * @return {int}
 */
int breakpoint_disable(struct Record* record, char* symbol, int pid) {

    if (!record->enable) {
        fprintf(stderr, "[breakpoint_disable]: This instruction not set breakpoint!\n");
        return -1;
    }

    long data = ptrace(PTRACE_PEEKDATA, pid, record->address, NULL);
    if (data == -1) {
        fprintf(stderr, "[breakpoint_disable]: Get symbol: %s instruction content failed! erron: %s", symbol, strerror(errno));
        return -1;
    }

    uint64_t restored_data = ((data & ~0xff) | record->save_data);
    if (ptrace(PTRACE_POKEDATA, pid, record->address, restored_data) == -1) {
        fprintf(stderr, "[breakpoint_disable]: Restored symbol: %s instruction failed! erron: %s", symbol, strerror(errno));
        return -1;
    }
    return 1;
}


/**
 * @brief: 初始化断点
 * @param {BreakPoint*} breakpoints
 * @param {int} pid
 * @description: 解析配置文件和ELF文件，获取所有需要设置断点的符号及对应地址，最后在符号的首条指令处设置断点
 * @return {*}
 */
void breakpoint_initialize(struct BreakPoint* breakpoints, char* elf_path, int pid) {
    
    strcpy(breakpoints->symbol, "hello");

    long load_address = GetLoadAddress(pid);
    long symbol_offset = GetSymbolAddress(elf_path, breakpoints->symbol);

    breakpoints->start.address = symbol_offset + load_address;
    printf("symbol start address: %lx\n", breakpoints->start.address);

    // 插入断点
    breakpoint_enable(&(breakpoints->start), breakpoints->symbol, pid);
}


/**
 * @brief: 跨越断点，避免程序在断点处无限循环
 * @param {BreakPoint*} breakpoint
 * @param {int} pid
 * @description: 
 * @return {*}
 */
int step_over_breakpoint(struct BreakPoint* breakpoints, int pid) {

    int status = 0;
    struct user_regs_struct regs;

    // 获取寄存器值
    if (ptrace(PTRACE_GETREGS, pid, 0, &regs) == -1) {
        fprintf(stderr, "[step_over_breakpoint]: Get regs failed! erron: %s", strerror(errno));
        return -1;
    }
    uint64_t breakpoint_previou_address = regs.rip - 1;
    uint8_t tail = 1;

    // 是符号首地址，且符号返回地址未设置断点
    if (breakpoints->start.address == breakpoint_previou_address)  {
        tail = -1;
        if (!breakpoints->end.enable) {
            long ret_address = ptrace(PTRACE_PEEKDATA, pid, regs.rsp, NULL);
            if (ret_address == -1) {
                fprintf(stderr, "[step_over_breakpoint]: Get return address failed! erron: %s", strerror(errno));
                return -1;
            }
            printf("now symbol: %s return address is: %lx\n",breakpoints->symbol, ret_address);
            breakpoints->end.address = ret_address;

            // 给符号返回地址设置断点
            breakpoint_enable(&(breakpoints->end), breakpoints->symbol, pid);
        }
    }

    // 恢复PC计数器，重新执行断点处的原本的指令
    regs.rip = breakpoint_previou_address;
    if (ptrace(PTRACE_SETREGS, pid, 0, &regs) == -1) {
        fprintf(stderr, "[step_over_breakpoint]: Set regs failed! erron: %s", strerror(errno));
        return -1;
    }

    struct Record * record = tail ? &(breakpoints->end) : &(breakpoints->start);

    // 禁止断点，单步执行到下一条指令，避免无限循环
    breakpoint_disable(record, breakpoints->symbol, pid);
    if (ptrace(PTRACE_SINGLESTEP, pid, 0, 0) == -1) {
        fprintf(stderr, "[step_over_breakpoint]: Single step failed! erron: %s", strerror(errno));
        return -1;
    }
    waitpid(pid, &status, 0);
    breakpoint_enable(record, breakpoints->symbol, pid);
}


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
            execvp(argv[1], argv + 1);
            FATAL("%s", strerror(errno));
    }

    waitpid(pid, &status, 0);
    ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_EXITKILL|PTRACE_O_TRACESYSGOOD);

    // 初始化断点结构
    struct BreakPoint breakpoint;
    memset(&breakpoint, 0, sizeof(breakpoint));
    breakpoint_initialize(&breakpoint, argv[1], pid);

    // 让tracee继续执行,直到触发断点
    ptrace(PTRACE_CONT, pid, 0, 0);
    waitpid(pid, &status, 0);
    if (WSTOPSIG(status) == 0x5) {
        printf("first!\n");
    }

    // 越过断点
    step_over_breakpoint(&breakpoint, pid);

    for (;;) {
        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1){
            printf("tracee exit 1!\n");
            break;
        }
        if (waitpid(pid, &status, 0) == -1) {
            printf("tracee exit 2!\n");
            exit(0);
        }
        if (WSTOPSIG(status) == 0x5) {
            printf("second in first wait!\n");
            break;
        }

        struct user_regs_struct regs;
        if (ptrace(PTRACE_GETREGS, pid, 0, &regs) == -1) {
            return -1;
        }
        long syscall = regs.orig_rax;
        fprintf(stderr, "%ld(0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)\n",
                syscall,
                (long)regs.rdi, (long)regs.rsi, (long)regs.rdx,
                (long)regs.r10, (long)regs.r8,  (long)regs.r9,(long)regs.rip);
                
        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1){
            printf("tracee exit 3!\n");
            exit(0);
        }
        if (waitpid(pid, &status, 0) == -1){ 
            printf("tracee exit 4!\n");
            exit(0);
        }
        if (WSTOPSIG(status) == 0x5) {
            printf("second in second wait!\n");
            break;
        }
    }
    // 越过断点
    step_over_breakpoint(&breakpoint, pid);
    ptrace(PTRACE_CONT, pid, 0, 0);
    
    if (WIFEXITED(status)) {
        printf("tracee exit normally! with exit code %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("tracee was terminated by signal %d\n",WTERMSIG(status));
    }
    return 0;
}


