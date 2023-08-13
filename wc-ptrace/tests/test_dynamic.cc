/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-08-12 16:57:28
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-12 17:00:46
 * @FilePath: /wc-intercept/wc-ptrace/tests/test_dynamic.cc
 * @Description: 测试动态更新后端处理逻辑
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