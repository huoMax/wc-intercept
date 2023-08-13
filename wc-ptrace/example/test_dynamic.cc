/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-08-13 17:28:53
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-13 17:29:22
 * @FilePath: /wc-intercept/wc-ptrace/example/test_dynamic.cc
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
        sleep(1);
    }
    return 0;
}