/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-07-28 21:12:03
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-07-28 21:14:06
 * @FilePath: /wc-intercept/wc-ptrace/learning/tmp/ll.cpp
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <stdio.h>
void syscall_1() {
    printf("syscall 1\n");
}

void syscall_2() {
    printf("syscall 2\n");
}

void syscall_start(int a) {
    if (a==1) {
        syscall_1();
    }
    if (a==2) {
        syscall_2();
    }
}

