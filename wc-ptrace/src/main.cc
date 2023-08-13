/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-07-24 23:21:32
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-13 14:48:06
 * @FilePath: /wc-intercept/wc-ptrace/src/main.cc
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include "elf.h"
#include "target.h"
#include "env.h"
#include "monitor.h"
#include "log.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <signal.h>

static bool flag_update_1 = false;
static bool flag_update_2 = false;

void signal_handler(int signum);

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cout << "usage: " << argv[0] << " file-name" << std::endl;
        return 0;
    }

    // 初始化环境变量模块
    wc::Env env;
    env.init(argc, argv);
    
    // 初始化配置模块
    std::string conf_root = env.get_config_path();
    wc::Config::ptr config(new wc::Config(conf_root));

    // 设置日志输出文件
    std::string logger_path = config->get_string("logger_path");
    if (logger_path.empty()) {
        logger_path = "logger.txt";
    }
    wc::Logger::ptr logger = WC_LOG_NAME("intercept");
    wc::FileLogAppender::ptr fileAppender(new wc::FileLogAppender(logger_path.c_str()));
    logger->addAppender(fileAppender);
    logger->setLevel(wc::LogLevel::INFO);

    // 初始化可行执行文件类
    wc::TraceeElf::ptr elf = nullptr;
    std::string tracee_path = env.get("e", "");
    if (tracee_path == "") {
        WC_LOG_DEBUG(logger) << "Can't found tracee, please check program input!" << std::endl;
        return 0;
    }
    try {
        elf.reset(new wc::TraceeElf(tracee_path));
        elf->initialize();
    } catch (std::exception& e) {
        std::cout << "Exception caught: " << e.what() << std::endl;
        return 0;
    }
    pid_t pid = fork();
    int status;
    switch (pid) {
        case -1:
            WC_LOG_DEBUG(logger) << strerror(errno) <<std::endl;
            return 0;
            exit(-1);
        case 0: 
            ptrace(PTRACE_TRACEME, 0, 0, 0);
            execvp(tracee_path.c_str(), argv + 1);
            WC_LOG_DEBUG(logger) << strerror(errno) <<std::endl;
            return 0;;
    }
    waitpid(pid, &status, 0);
    ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_EXITKILL|PTRACE_O_TRACESYSGOOD);
    
    // 初始化监视器
    wc::Monitor::ptr monitor = nullptr;
    try {
        monitor.reset(new wc::Monitor(config, elf, pid));
        monitor->set_config_path(conf_root);
    } catch (std::exception& e) {
        std::cout << "Exception caught: " << e.what() << std::endl;
        return 0;
    }

    // 初始化信号处理函数
    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, signal_handler);
    std::cout << "tracee pid: " << getpid() << std::endl;

    // 在入口地址设置断点
    monitor->entry(elf);
    monitor->wait_no_syscall(config, elf, flag_update_1, flag_update_2);
    
    return 0;
}

void signal_handler(int signum) {
    if (signum == SIGUSR1) {
        flag_update_1 = true;
    } else if (signum == SIGUSR2) {
        std::cout << "accept siguser2" <<std::endl;
        flag_update_2 = true;
    }
}