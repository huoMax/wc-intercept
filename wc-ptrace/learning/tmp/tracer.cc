/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-07-24 23:21:32
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-07-29 22:50:23
 * @FilePath: /wc-intercept/wc-ptrace/learning/tmp/tracer.cc
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/ptrace.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

long load_address = 0x0;

int
main(int argc, char **argv)
{
    if (argc < 2)
        exit(-1);

    pid_t pid = fork();
    int status;
    switch (pid) {
        case -1:
            exit(-1);
        case 0: 
            ptrace(PTRACE_TRACEME, 0, 0, 0);
            std::cout <<"test child!\n" <<std::endl;
            execvp(argv[1], argv + 1);
            exit(-1);
    }
    
    waitpid(pid, &status, 0);
    // 获取加载地址
    std::cout << "tracer" << std::endl;
    char buf[100];
    scanf("%s", buf);

    ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_EXITKILL|PTRACE_O_TRACESYSGOOD);
    ptrace(PTRACE_CONT, pid, 0, 0);
    return 0;
}
    