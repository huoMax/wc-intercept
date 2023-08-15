/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-07-27 20:33:23
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-15 15:27:57
 * @FilePath: /wc-intercept/wc-ptrace/src/env.cc
 * @Description: 环境变量类实现
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include "env.h"
#include <unistd.h>
#include <stdlib.h>
#include <memory>
#include <iostream>
#include <iomanip>
#include <string.h>

namespace wc {
    
bool Env::init(int argc, char **argv) {
    char link[1024] = {0};
    char path[1024] = {0};
    sprintf(link, "/proc/%d/exe", getpid());
    readlink(link, path, sizeof(path));
    m_exe = path;

    auto pos = m_exe.find_last_of("/");
    m_cwd    = m_exe.substr(0, pos) + "/";

    // 命令行参数解析
    m_program = argv[0];
    const char *now_key = nullptr;
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if (strlen(argv[i]) > 1) {
                if (now_key) {
                    add(now_key, "");
                }
                now_key = argv[i] + 1;
            } else {
                std::cout << "[logger]: " <<  "invalid arg idx=" << i
                                          << " val=" << argv[i]; 
                return false;
            }
        } else {
            if (now_key) {
                add(now_key, argv[i]);
                now_key = nullptr;
            } else {
                std::cout << "[logger]: " << "invalid arg idx=" << i
                                          << " val=" << argv[i];
                return false;
            }
        }
    }
    if (now_key) {
        add(now_key, "");
    }
    return true;
}

void Env::add(const std::string &key, const std::string &val) {
    m_args[key] = val;
}

bool Env::has(const std::string &key) {
    auto it = m_args.find(key);
    return it != m_args.end();
}

void Env::del(const std::string &key) {
    m_args.erase(key);
}

std::string Env::get(const std::string &key, const std::string &default_value) {
    auto it = m_args.find(key);
    if (it != m_args.end()) {
        return it->second;
    } else return default_value;
}

void Env::add_help(const std::string& key, const std::string& desc) {
    remove_help(key);
    m_helps.push_back(std::make_pair(key, desc));
}

void Env::remove_help(const std::string& key) {
    for (auto it = m_helps.begin(); it != m_helps.end(); ) {
        if (it->first == key) {
            it = m_helps.erase(it);
        } else {
            ++it;
        }
    }
}

void Env::print_help() {
    std::cout << "Usage: " << m_program << " [options]" << std::endl;
    for (auto &i : m_helps) {
        std::cout << std::setw(5) << "-" << i.first << " : " << i.second << std::endl;
    }
}

bool Env::set_env(const std::string &key, const std::string &val) {
    return !setenv(key.c_str(), val.c_str(), 1);
}

std::string Env::get_env(const std::string &key, const std::string &default_value) {
    const char *v = getenv(key.c_str());
    if (v == nullptr) {
        return default_value;
    }
    return v;
}

std::string Env::get_absolute_path(const std::string &path) const {
    if (path.empty()) {
        return "/";
    }
    if (path[0] == '/') {
        return path;
    }
    return m_cwd + path;
}

std::string Env::get_config_path() {
    return get_absolute_path(get("c", "conf/config.json"));
}

}
