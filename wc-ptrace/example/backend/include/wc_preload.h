#ifndef __WC_PRELOAD_H_
#define __WC_PERLOAD_H_
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

/* 返回值不为指针 */
#define FUNC(name, ret_type,...) \
    typedef ret_type ((*wc_##name##_handle)(__VA_ARGS__)); \
    wc_##name##_handle wc_##name = NULL;

/* 返回值为指针 */
#define FUNC_PTR(name, ret_type, ...) \
    typedef ret_type (*(*wc_##name##_handle)(__VA_ARGS__)); \
    wc_##name##_handle wc_##name = NULL;

/* 初始化 */
#define INITIAL(name) \
    if (wc_##name == NULL) { \
        wc_##name = (wc_##name##_handle)dlsym(RTLD_NEXT, #name); \
    }

#endif