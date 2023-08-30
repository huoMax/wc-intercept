/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-08-18 17:39:08
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-18 19:01:54
 * @FilePath: /wc-intercept/test_1/mvee_1.c
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
int location = 0;

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
    memcpy(&(data[location].content), regs, sizeof(struct user_regs_struct));
    data[location].flag = 1;
    while (1) {
        if (data[1].flag==1 && data[2].flag==1) break;
    }
    int len = data[location].content.rdx;
    printf("pid: %d, len: %d\n", getpid(), len);
    for (int i=0; i<3; ++i) {
        if (i == location) {
            continue;
        }
        if (data[i].content.rdx != len) {
            printf("Write Content not Consistent!\n");
            exit(-1);
        }
    }
    data[location].flag = 0;
}
