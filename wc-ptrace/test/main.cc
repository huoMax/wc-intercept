/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-08-13 20:38:56
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-13 20:44:32
 * @FilePath: /wc-intercept/wc-ptrace/test/main.cc
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include "env.h"
#include <iostream>

int main(int argc, char** argv) {
    // 初始化环境变量模块
    wc::Env env;
    env.init(argc, argv);

    std::cout << env.has("l") << std::endl;
}