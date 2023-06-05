/*
 * @Author: huomax
 * @Date: 2023-05-21 03:31:56
 * @LastEditors: huomax
 * @LastEditTime: 2023-06-06 06:12:44
 * @FilePath: /wgk/wc-intercept/ptrace/example/tracee.c
 * @Description: 测试程序（tracee），输出hello world!
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void hello1() {
    FILE* fp;
    fp = fopen("./example/hello1.txt", "r");
    fclose(fp);
}

void hello2() {
    printf("hello world!\n");
}

void hello3() {
    char hello[10];
    strcpy(hello, "hello");
    printf("%s\n", hello);
}

int main(void)
{
    hello1();
    hello2();
    hello3();
    printf("I am tracee, now exit!\n");
    return 0;
}
