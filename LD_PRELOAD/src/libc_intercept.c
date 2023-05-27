#include "../include/tackle_register.h"

int rand() {
    int (*_libc_rand)() = LIBC_SYMBOL_ADDR(rand);
    return 42;
}