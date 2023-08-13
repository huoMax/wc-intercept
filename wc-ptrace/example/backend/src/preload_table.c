/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-08-13 17:43:28
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-13 17:43:55
 * @FilePath: /wc-intercept/wc-ptrace/example/backend/src/preload_table.c
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */

#include "../include/wc_preload.h"
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

FUNC_PTR(malloc, void, size_t)
FUNC(free, void, void*)
FUNC_PTR(fopen, FILE, const char*, const char*)
FUNC(fclose, int, FILE*)
FUNC_PTR(fgets, char, char*, int, FILE*)
FUNC(fwrite, size_t, const void*, size_t, size_t, FILE*)
FUNC(rand, int)
FUNC_PTR(memset, void, void*, int, size_t)

void* malloc(size_t size) {
    INITIAL(malloc)
    return wc_malloc(size);
}

void free(void* ptr) {
    INITIAL(free)
    wc_free(ptr);
}

FILE* fopen(const char* filename, const char* mode) {
    INITIAL(fopen)
    return wc_fopen(filename, mode);
}

int fclose(FILE* stream) {
    INITIAL(fclose)
    return wc_fclose(stream);
}

char* fgets(char* str, int num, FILE* stream) {
    INITIAL(fgets)
    return wc_fgets(str, num, stream);
}

size_t fwrite(const void* ptr, size_t size, size_t count, FILE* stream) {
    INITIAL(fwrite)
    return wc_fwrite(ptr, size, count, stream);
}

void* memset(void* ptr, int value, size_t num) {
    INITIAL(memset)
    return wc_memset(ptr, value, num);
}

int rand() {
    INITIAL(rand)
    return wc_rand();
}
