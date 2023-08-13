/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-08-01 15:08:13
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-10 21:41:08
 * @FilePath: /wc-intercept/wc-ptrace/learning/tt/main.cc
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

int main() {
    printf("hello\n");
    FILE *input, *output;
    struct timespec start = {0};
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    printf("hello\n");
    
    int count = 100000;
    while (count--) {
        int num = rand();
        printf("%d\n", num);
        char *buffer = (char*)malloc(sizeof(char)*256);
        memset(buffer, '1', sizeof(char)*256);
        // printf("%c, %c\n", buffer[200], buffer[0]);
        input = fopen("read.txt", "r, ccs=GBK");
        output = fopen("write.txt", "a, ccs=GBK");
        fgets(buffer, 256, input);
        fwrite(buffer, sizeof(char), 256, output);
        fclose(input);
        fclose(output);
        free(buffer);
    }

    struct timespec end = {0};
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    printf("cost msec: %ld\n", (end.tv_sec * 1000 + end.tv_nsec / 1000000)-(start.tv_sec * 1000 + start.tv_nsec / 1000000));
    return 0;
}