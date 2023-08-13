/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-08-10 19:37:59
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-10 21:50:22
 * @FilePath: /wc-intercept/wc-ptrace/learning/tt/back.c
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#define FUNC_PTR(name, ret_type,...) typedef ret_type ((*wc_##name)(__VA_ARGS__)); \
                            ret_type name(__VA_ARGS__) { \
                            wc_##name raw = (wc_##name)dlsym(RTLD_NEXT, #name); \
                            printf("enter\n"); \
                            ret_type tmp = raw(__VA_ARGS__) %100; \
                            printf("exit\n"); \
                            return  tmp;}
#define FUNC_PTR_PTR(name, ret_type, ...) typedef ret_type (*(*wc_##name)(__VA_ARGS__)); \
                            ret_type* name(__VA_ARGS__) { \
                            wc_##name raw = (wc_##name)dlsym(RTLD_NEXT, #name); \
                            return raw(__VA_ARGS__); }

FUNC_PTR(rand, int)

// int rand() {
//     FUNC_PTR(wc_rand, int);
//     wc_rand raw = (wc_rand)dlsym(RTLD_NEXT, "rand");
//     raw();
//     // printf("hello\n");
//     return raw() % 100 - 101;
// }

// void* malloc(size_t size) {
//     typedef void (*(*malloc_handle)(size_t));
//     // FUNC_PTR_PTR(malloc_handle, void, size_t);
//     malloc_handle raw = (malloc_handle)dlsym(RTLD_NEXT, "malloc");
//     // printf("hello\n");
//     return raw(size);
// }

// void free(void *ptr) {
//     typedef void (*free_handle)(void *);
//     free_handle raw = (free_handle)dlsym(RTLD_NEXT, "free");
//     raw(ptr);
// }

// FILE *fopen(const char *filename, const char *mode) {
//     typedef FILE (*(*raw_handle)(const char *, const char *));
//     raw_handle raw = (raw_handle)dlsym(RTLD_NEXT, "fopen");
//     return raw(filename, mode);
// }

// char *fgets(char *str, int num, FILE *stream) {
//     typedef char (*(*raw_handle)(char *, int, FILE *));
//     raw_handle raw = (raw_handle)dlsym(RTLD_NEXT, "fgets");
//     return raw(str, num, stream);
// }

// size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream) {
//     typedef size_t (*raw_handle)(const void *, size_t, size_t, FILE*);
//     raw_handle raw = (raw_handle)dlsym(RTLD_NEXT, "fwrite");
//     printf("hello\n");
//     return raw(ptr, size, count, stream);
// }

// int fclose(FILE *stream) {
//     typedef size_t (*raw_handle)(FILE *);
//     raw_handle raw = (raw_handle)dlsym(RTLD_NEXT, "fclose");
//     return raw(stream);
// }
