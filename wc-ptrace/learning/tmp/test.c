/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-07-29 10:44:17
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-07-29 10:44:33
 * @FilePath: /wc-intercept/wc-ptrace/learning/tmp/test.c
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <stdio.h>
int main(int argc, char* argv[]){
    int i = 0;
    int arr[3] = {0};
    for(; i<=3; i++){
        arr[i] = 0;
        printf("hello world\n");
    }
    return 0;
}