/*
 * @Author: huomax
 * @Date: 2023-06-06 18:22:05
 * @LastEditors: huomax
 * @LastEditTime: 2023-06-07 04:57:37
 * @FilePath: /wgk/wc-intercept/LD_PRELOAD/test/test.c
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

int pid;

int parser_config(char* config_path, char* ld_path) {

    FILE *fp = NULL;
    fp = fopen(config_path, "r");
    if (fp == NULL) {
        fprintf(stderr, "[parser_config]: open config file failure, please check config file!\n");
        return -1;
    }

    char line[256] = "";
    while (fgets(line, 256, fp) != NULL) {
        if (strncmp(line, "[LD_PRELOAD]", 12) == 0){
            memset(line, 0, sizeof(line));
            while (fgets(line, 256, fp) != NULL) {
                if (strncmp(line, "LD_PRELOAD=", 11) == 0) {
                    strcpy(ld_path, line);
                }
                memset(line, 0, sizeof(line));
            }
            break;
        }
        memset(line, 0, sizeof(line));
    }

    fclose(fp);
    return 1;
}

int main()
{
    char ld_path[256];
    if (parser_config("../ini.config", ld_path) == -1) {
        printf("youwenti!\n");
    }
    printf("%s\n", ld_path);

    return 0;
}


