/*
 * @Author: huomax
 * @Date: 2023-05-21 05:07:28
 * @LastEditors: huomax
 * @LastEditTime: 2023-05-27 05:18:44
 * @FilePath: /wgk/wc-intercept/ptrace/sample/test.c
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <stdio.h>
#include <string.h>

void PeekData(int size, char* dest) {
    char buf[50] = "123456789abcdefgheijhlmnopqrst";

    int index = 0;
    while (size > 0) {
        if (size < 8) {
            memcpy(dest+index, buf+index, size);
            break;
        }
        memcpy(dest+index, buf+index, 8);
        size -= 8;
        index += 8;
    }
}

int main(int argc, char **argv) {

    char test[50] = "";
    PeekData(20, test);
    printf("%s\n", test);
    return 0;
}