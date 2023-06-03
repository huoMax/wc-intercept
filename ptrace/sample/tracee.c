/*
 * @Author: huomax
 * @Date: 2023-05-21 03:31:56
 * @LastEditors: huomax
 * @LastEditTime: 2023-06-04 05:07:08
 * @FilePath: /wgk/wc-intercept/ptrace/sample/tracee.c
 * @Description: 测试程序（tracee），输出hello world!
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <stdio.h>

void hello() {
    printf("hello!\n");
}

int main(void)
{
    hello();
    printf("world!\n");
    return 0;
}
