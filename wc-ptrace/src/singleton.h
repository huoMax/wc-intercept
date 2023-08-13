/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-07-26 22:15:48
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-01 15:44:37
 * @FilePath: /wc-intercept/wc-ptrace/src/singleton.h
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#ifndef __WC_INTERCEPT_SINGLETON__
#define __WC_INTERCEPT_SINGLETON__
#include <memory>

namespace wc {
/**
 * @brief: 单例模式（懒汉模式）封装，返回裸指针
 */
template<class T>
class Singleton {
public:
    static T* GetInstance() {
        static T v;
        return &v;
    }

    Singleton(const Singleton&) = delete;
    void operator=(const Singleton&) = delete;
private:
    Singleton() {}
    ~Singleton() {}
};

/**
 * @brief: 单例模式（懒汉模式）封装，返回共享指针
 */
template<class T>
class SingletonPtr {
public:
    static std::shared_ptr<T> GetInstance() {
        static std::shared_ptr<T> v(new T);
        return v;
    }

    SingletonPtr(const SingletonPtr&) = delete;
    void operator=(const SingletonPtr&) = delete;
private:
    SingletonPtr() {}
    ~SingletonPtr() {}
};

}
#endif