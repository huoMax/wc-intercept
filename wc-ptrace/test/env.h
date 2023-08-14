/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-07-26 20:56:26
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-13 20:42:05
 * @FilePath: /wc-intercept/wc-ptrace/test/env.h
 * @Description: 环境变量模块封装
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#ifndef __WC_INTERCEPT_ENV_H__
#define __WC_INTERCEPT_ENV_H__

#include <map>
#include <vector>

namespace wc
{
class Env {
public:
    /**
     * @brief: 初始化，包括程序名称与路径，解析命令行选项和参数
     * @param {int} argc
     * @param {char} *
     */
    bool init(int argc, char **argv);

    /**
     * @brief: 添加自定义环境变量
     * @param {string&} key
     * @param {string&} value
     */    
    void add(const std::string& key, const std::string& value);

    /**
     * @brief: 判断是否存在键值为key的环境变量
     * @param {string&} key
     */    
    bool has(const std::string& key);

    /**
     * @brief: 删除键值为key的环境变量
     * @param {string&} key
     */    
    void del(const std::string& key);

    /**
     * @brief: 获取键值为Key的自定义环境变量，如果未找到，则返回默认值
     * @param {string&} key
     * @param {string&} default_vaule
     */    
    std::string get(const std::string& key, const std::string& default_vaule="");

    /**
     * @brief: 为键值为key的环境变量添加帮助解释
     * @param {string&} key
     * @param {string&} desc
     */    
    void add_help(const std::string& key, const std::string& desc);

    /**
     * @brief: 移除键值为key的环境变量帮助选项
     * @param {string&} key
     */    
    void remove_help(const std::string& key);

    /**
     * @brief: 打印帮助信息
     */    
    void print_help();

    /**
     * @brief: 获取exe完整路径
     */    
    const std::string& get_exe() const { return m_exe; }

    /**
     * @brief: 获取当前路径，从main函数的argv[0]获取，以/结尾
     */    
    const std::string& get_cwd() const { return m_cwd; }

    /**
     * @brief: 设置系统环境变量
     * @param {string&} key
     * @param {string&} val
     */    
    bool set_env(const std::string& key, const std::string& val);

    /**
     * @brief: 获取系统环境环境变量
     * @param {string&} key
     * @param {string&} default_value
     */    
    std::string get_env(const std::string& key, const std::string& default_value="");

    /**
     * @brief: 提供一个相对于当前的路径，返回这个path的绝对路径
     * @param {string&} path
     */    
    std::string get_absolute_path(const std::string& path) const;

    /**
     * @brief: 获取配置文件路径，配置文件路径通过命令行-c选项指定，默认为当前目录下的conf文件夹
     */    
    std::string get_config_path();

private:
    // 程序完整路径名，从/proc/$pid/exe软件链接中获取
    std::string m_exe;
    // 当前路径，从argv[0]中获取
    std::string m_cwd;
    // 程序名，也即是argv[0]
    std::string m_program;

    // 存储程序自定义的环境变量
    std::map<std::string, std::string> m_args;
    // 存储帮助选项与描述
    std::vector<std::pair<std::string, std::string>> m_helps;
};

} // namespace wc

#endif