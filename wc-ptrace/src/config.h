/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-07-26 23:16:15
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-15 18:30:13
 * @FilePath: /wc-intercept/wc-ptrace/src/config.h
 * @Description: 配置模块类封装
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#ifndef __WC__INTERCEPT_CONFIG_H__
#define __WC__INTERCEPT_CONFIG_H__

#include <fstream>
#include <memory>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <json/json.h>

// 支持的拦截信息选项
#define SYSCALL_NAME        "syscall_id"    // 系统调用号
#define FUNC_NAME           "func_name"     // 函数名
#define FREQUENCY           "frequency"     // 拦截频率，未使用
#define BACKEND_ENTER       "back_enter"    // 函数/系统调用入口的后端处理函数名称
#define BACKEND_EXIT        "back_exit"     // 函数/系统调用返回的后端处理函数名称
#define LD_PRELOAD          "preload"       // LD_PRELOAD支持，未使用

// 配置文件选项
#define INTERCEPT_FUNC      "funcs"         // 所有的函数拦截目标
#define INTERCEPT_SYSCALL   "syscalls"      // 所有的系统调用拦截目标
#define CONFIG_LIBRARY      "library_path"  // 指定后端处理逻辑库路径
#define CONFIG_LD_PRELOAD   "ld_preload"    // 指定的预加载库路径

#define CONFIG_PATH     "config.json"       // 默认的配置文件名

namespace wc {

/**
 * @brief: 配置文件基类
 */
class Config {
public:
    friend class Monitor;
    typedef std::shared_ptr<Config> ptr;
    /**
     * @brief: 构造函数，用于打开配置文件并读取json文件，读取失败抛出std::logic_error
     */    
    Config(const std::string& conf_path) : m_conf_path(conf_path) {
        std::ifstream ifs;
        ifs.open(m_conf_path);
        if (!ifs.is_open()) {
            std::logic_error("Can't open conf file!");
        }
        Json::Reader reader;
        if (!reader.parse(ifs, m_values, false)) {
            std::logic_error("Can't read conf file!");
        }
        ifs.close();
    }

    /**
     * @brief: 更新配置选项，重新读取配置文件，虚函数
     */    
    bool update();

    /**
     * @brief: 获取指定键值的配置选项值，返回string类型
     */    
    std::string get_string(const std::string& key);

    /**
     * @brief: 获取指定键值的配置选项值，返回int类型，读取失败返回0
     */    
    int get_int(const std::string& key);
    
public:
    // 读取到的json键值
    Json::Value m_values;
    // 配置文件名
    std::string m_conf_path;
};

}
#endif