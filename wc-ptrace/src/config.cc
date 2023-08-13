/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-07-27 20:51:10
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-12 21:16:30
 * @FilePath: /wc-intercept/wc-ptrace/src/config.cc
 * @Description: 配置模块类实现
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include "config.h"
#include "log.h"
#include <iostream>

static wc::Logger::ptr logger = WC_LOG_NAME("intercept");

namespace wc {

std::string Config::get_string(const std::string& key) {
    if (m_values.isMember(key)) {
        return m_values[key].asString();
    }
    return "";
}

int Config::get_int(const std::string& key) {
    if (m_values.isMember(key)) {
        return m_values[key].asInt();
    }
    return 0;
}

bool Config::update() {
    std::ifstream ifs;
    ifs.open(m_conf_path);
    if (!ifs.is_open()) {
        WC_LOG_ERROR(logger) << "[Config::update] Can't open conf file" << std::endl;
        return false;
    }
    Json::Reader reader;
    if (!reader.parse(ifs, m_values, false)) {
        WC_LOG_ERROR(logger) << "[Config::update] Can't read conf file!" << std::endl;
        return false;
    }
    ifs.close();
    return true;
}

}