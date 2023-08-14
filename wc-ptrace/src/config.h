/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-07-26 23:16:15
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-13 20:27:09
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

// 支持的拦截配置选项
#define SYSCALL_NAME        "syscall_id"
#define FUNC_NAME           "func_name"
#define FREQUENCY           "frequency"
#define BACKEND_ENTER       "back_enter"
#define BACKEND_EXIT        "back_exit"
#define INTERCEPT_FUNC      "funcs"
#define INTERCEPT_SYSCALL   "syscalls"
#define LD_PRELOAD          "preload"

// 默认的配置文件名
#define CONFIG_PATH     "config.json"

// 配置选项
#define CONFIG_LIBRARY      "library_path"
#define CONFIG_LD_PRELOAD   "ld_preload"

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