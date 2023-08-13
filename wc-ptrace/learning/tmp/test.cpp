/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-07-28 21:11:29
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-07-28 21:18:45
 * @FilePath: /wc-intercept/wc-ptrace/learning/tmp/test.cpp
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <dlfcn.h>
#include <iostream>

int main() {
    // 加载动态库
    void* handle = dlopen("./ll.so", RTLD_LAZY);
    if (!handle) {
        std::cerr << "Cannot open library: " << dlerror() << std::endl;
        return 1;
    }

    // 获取函数指针
    typedef void (*syscall_start)(int);
    syscall_start my_func = (syscall_start)dlsym(handle, "syscall_start");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "Cannot load symbol: " << dlsym_error << std::endl;
        dlclose(handle);
        return 1;
    }

    // 调用函数
    my_func(1);
    my_func(2);

    // 卸载动态库
    dlclose(handle);
    return 0;
}
