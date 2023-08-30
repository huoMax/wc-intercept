/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-08-18 17:39:08
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-18 18:39:23
 * @FilePath: /wc-intercept/wc-ptrace/example/backend/src/mvee.c
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/user.h>

struct MVEE {
    int flag;
    struct user_regs_struct content;
};

void * shared_memory = NULL;
int location = -1;

void Initial_Shared(int count) {
    int key = 19981012;
    int shmid = shmget(key, sizeof(struct MVEE)*count, 0644 | IPC_CREAT);
    if (shmid == -1) {
        printf("get shared memory failed!\n");
        exit(-1);
    }
    shared_memory = shmat(shmid, (void *)0, 0);
    if (shared_memory == (char *)(-1)) {
        printf("shmat failed\n");
        exit(-1);
    }
}

void mvee_fgets(struct user_regs_struct* regs, int pid) {
    if (shared_memory == NULL) {
        Initial_Shared(3);
    }
    struct MVEE *data = (struct MVEE*)shared_memory;
    if (location == -1) {
        for (int i=0; i<2; ++i) {
            if (data[i].flag != 1) {
                data[i].flag = 1;
                location = i;
            }
        }
    }
    if (location == -1) {
        printf("error found location!\n");
        exit(-1);
    }
    memcpy(&(data[location].content), regs, sizeof(struct user_regs_struct));
    int flag = 1;
    while (flag == 3) {
        for (int i=0; i<3; ++i) {
            if (i==location) continue;
            else {
                if (data[i].flag == 1) {
                    ++flag;
                }
            }
        }
    }
    int len = data[location].content.rdx;
    for (int i=1; i<3; ++i) {
        if (len != data[i].content.rdx) {
            printf("Write Content not Consistent!\n");
            exit(-1);
        }
    }
    memset(shared_memory, 0, sizeof(struct MVEE)*3);
}
