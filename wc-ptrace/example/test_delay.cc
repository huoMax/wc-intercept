/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-08-14 10:52:31
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-14 10:53:59
 * @FilePath: /wc-intercept/wc-ptrace/test_delay.cc
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main() {
    int count = 180;

    while (count--) {
        int num = rand();
        printf("num: %d\n", num);
        sleep(1);
    }
    return 0;
}