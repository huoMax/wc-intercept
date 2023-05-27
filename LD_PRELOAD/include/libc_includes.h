#ifndef __LIBC_INCLUDES__
#define __LIBC_INCLUDES__

#define _GNU_SOURCE
#include <dlfcn.h>

#define LIBC_SYMBOL_ADDR(symbol) dlsym(RTLD_NEXT, #symbol)
#define LIBC_DECLARE(symbol, return_type, ...) return_type symbol(__VA_ARGS__);

#endif
