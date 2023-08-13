/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-07-09 01:54:05
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-07-09 04:48:34
 * @FilePath: /wc-intercept/wc-ptrace/learning/delay_bind/tracee.c
 * @Description: 用于解析延迟绑定
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <stdio.h>
#include <stdlib.h>

int var_global = 9;

void test(){
    int a=0, b=1;
    printf("test\n");
}

int main () {
    printf("hello world, I will rand a: %d\n", rand());
    printf("byby, I will rand a: %d\n", rand());
    test();
    return 0;
}