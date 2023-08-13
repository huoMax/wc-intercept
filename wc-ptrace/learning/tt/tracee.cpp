/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-08-06 11:30:55
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-06 11:34:40
 * @FilePath: /wc-intercept/wc-ptrace/learning/tt/tracee.cpp
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <stdio.h>

int test(int a, int b) {
    return a+b;
}

int main() {
    printf("hell world: %s\n", "wgk");
    test(1, 2);
    return 0;
}