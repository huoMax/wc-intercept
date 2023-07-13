/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-07-14 00:49:23
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-07-14 00:50:42
 * @FilePath: /wc-intercept/wc-ptrace/test.cpp
 * @Description: 
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
    return a+b;
}

int main(void) {
    test1();
    
    test2(2, 3);

    test3(3, 4);

    rand();

    return 0;
}