/*
 * @Author: huomax
 * @Date: 2023-05-19 04:31:12
 * @LastEditors: huomax
 * @LastEditTime: 2023-06-06 06:18:47
 * @FilePath: /wgk/wc-intercept/LD_PRELOAD/include/libc_includes.h
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#ifndef __LIBC_INCLUDES_
#define __LIBC_INCLUDES_

#define _GNU_SOURCE
#include <dlfcn.h>

#define LIBC_SYMBOL_ADDR(symbol) dlsym(RTLD_NEXT, #symbol)
#define LIBC_DECLARE(symbol, return_type, ...) return_type symbol(__VA_ARGS__);

#endif
