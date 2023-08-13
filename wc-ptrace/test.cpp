/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-07-14 00:49:23
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-07-25 14:14:49
 * @FilePath: /wc-intercept/wc-ptrace/test.cpp
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <signal.h>
#include <unistd.h>
#include <yaml-cpp/yaml.h>
#include <iostream>

void test1() {
    printf("test1\n");
}

void test2(int a, int b) {
    printf("test2\n");
}

int test3(int a, int b) {
    printf("test3: %d, %d\n", a, b);
    char* space = (char*)malloc(sizeof(char)*20);
    free(space);
    return a+b;
}

void test4(std::string a) {
    std::cout << a << std::endl;
}

int main(void) {
    // raise(SIGSTOP);
    test1();
    
    test2(2, 3);

    test3(3, 4);

    rand();

    return 0;
}