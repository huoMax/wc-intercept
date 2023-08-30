/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-08-18 17:52:37
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-18 17:53:38
 * @FilePath: /wc-intercept/wc-ptrace/example/test_mvee.cc
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {

    FILE *input, *output;
    
    int count = 300;
    while (count--) {
        char *buffer = (char*)malloc(sizeof(char)*256);
        memset(buffer, '\n', sizeof(char)*256);
        input = fopen("read.txt", "r, ccs=GBK");
        output = fopen("write.txt", "a, ccs=GBK");
        fgets(buffer, 256, input);
        fwrite(buffer, sizeof(char), strlen(buffer), output);
        fclose(input);
        fclose(output);
        free(buffer);
        sleep(1);
    }

    return 0;
}