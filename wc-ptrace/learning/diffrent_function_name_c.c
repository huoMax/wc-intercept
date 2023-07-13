/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-07-04 03:43:40
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-07-04 04:16:29
 * @FilePath: /wc-intercept/wc-ptrace/learning/diffrent_function_name_c.c
 * @Description: 简单示例，用于观察C语言符号表中的函数修饰规则
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <stdio.h>
#include <stdlib.h>

void test1() {
    printf("test1\n");
}

void test2(int a, int b) {
    printf("test2\n");
}

int test3(int a, int b) {
    printf("test3: %d, %d\n", a, b);
}

int main() {
    test1();
    
    test2(2, 3);

    test3(3, 4);

    rand();

    return 0;
}