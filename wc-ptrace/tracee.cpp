/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-07-18 04:58:54
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-06 19:08:23
 * @FilePath: /wc-intercept/wc-ptrace/tracee.cpp
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int main() {
    printf("hello world!\n");
    char * test = (char*)malloc(sizeof(char)*20);
    printf("test address: %lx, %lx\n", &test, test);
    free(test);
    rand();
    printf("byebye!\n");
    return 0;
}