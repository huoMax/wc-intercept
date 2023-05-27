/*
 * @Author: huomax
 * @Date: 2023-05-21 03:31:56
 * @LastEditors: huomax
 * @LastEditTime: 2023-05-26 20:06:35
 * @FilePath: /wgk/wc-intercept/ptrace/sample/tracee.c
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int hello(long a) {
    printf("hello!\n");
    return 1;
}

int main(void)
{
    char buf[8] = "1234567";
    long a = 0x12345678;
    hello(a);
    return 0;
}
