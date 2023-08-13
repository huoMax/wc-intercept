/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-08-08 14:28:44
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-10 11:35:06
 * @FilePath: /wc-intercept/wc-ptrace/tests/test_nrw.cc
 * @Description: 测试非读写系统调用
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

int main() {
    int count = 100000;
    struct timespec start = {0};
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    while (count--) {
        int num = rand() % 100;
        char* buf = (char*)malloc(sizeof(char)*num);
        memset(buf, '\n', num*sizeof(char));
        int pid = getpid();
        free(buf);
        if (count % 10000 == 0) {
            printf("hello\n");
        }
    }

    struct timespec end = {0};
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    printf("cost msec: %ld\n", (end.tv_sec * 1000 + end.tv_nsec / 1000000)-(start.tv_sec * 1000 + start.tv_nsec / 1000000));
    return 0;
}