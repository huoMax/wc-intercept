/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-08-06 18:24:04
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-06 18:32:09
 * @FilePath: /wc-intercept/wc-ptrace/library/test.cc
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <dlfcn.h>
#include <string>

int main() {
    typedef void (*backend_handle)(struct user_regs_struct*, int);
    void * m_library = nullptr;
    std::string backend_name = "sys_write_enter";
    m_library = dlopen("/home/huomax/wgk/wc-intercept/wc-ptrace/library/backend_table.so", RTLD_LAZY);
    backend_handle handle = (backend_handle)dlsym(m_library, backend_name.c_str());
    if (handle == NULL) {
        printf("cao!\n");
    }
    dlclose(m_library);
    return 0;
}