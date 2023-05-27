/*
 * pack_random.c
 */
#include <stdio.h>
#include <dlfcn.h>

int rand() {
    void *libc_handle = dlopen("libc.so.6", RTLD_NOW);
    if (libc_handle == NULL) {
        fprintf(stderr, "Error: %s\n", dlerror);
        return 1;
    }

    int (*libc_rand)() = dlsym(libc_handle, "rand");
    if (libc_rand == NULL) {
        fprintf(stderr, "Error: %s\n", dlerror);
        dlclose(libc_handle);
        return 1;
    }
    
    fprintf(stdout,"This is a pack function!\n");

    return libc_rand();
}
