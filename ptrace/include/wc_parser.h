/*
 * @Author: huomax
 * @Date: 2023-06-05 06:41:02
 * @LastEditors: huomax
 * @LastEditTime: 2023-06-06 04:24:06
 * @FilePath: /wgk/wc-intercept/ptrace/include/wc_parser.h
 * @Description: 断点相关函数定义
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */

#ifndef __WC_BREAKPOINT_
#define __WC_BREAKPOINT_

#define MAX_READ_LINE 256

#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

/* POSIX */
#include <unistd.h>
#include <stdint.h>
#include <sys/user.h>
#include <sys/wait.h>

/* Linux */
#include <syscall.h>
#include <sys/ptrace.h>

/* libelf */
#include <libelf.h>
#include <gelf.h>

struct Record {
    uint64_t address;   // 插入断点的地址
    uint8_t save_data;  // 被中断指令替换的内容
    uint8_t enable;     // 中断是否已设置
};

struct BreakPoint {
    char symbol[64];        // 符号名称
    struct Record head;     // 首地址断点相关
    struct Record ret;   // 返回地址断点相关
};


/**
 * @brief: 读取配置文件，初始化断点信息
 * @description: 从配置文件中读取要设置断点的符号，创建并初始化断点结构体数组
 * @return {struct BreakPoint*}
 */
struct BreakPoint* parser_config(char* config_path, int* ret_count);


/**
 * @brief: 插入断点
 * @param {Record*} record
 * @param {char *} symbol
 * @param {int} pid
 * @description: 替换指定地址指令，插入断点，将被替换指令内容保存在record.save_data中
 * @return {int} 失败则返回-1
 */
int breakpoint_enable(struct Record* record, char * symbol, int pid);


/**
 * @brief: 禁止断点
 * @param {Record*} record
 * @param {char*} symbol
 * @param {int} pid
 * @description: 使用record.save_data恢复指令内容，剔除断点
 * @return {int} 失败则返回-1
 */
int breakpoint_disable(struct Record* record, char* symbol, int pid);


/**
 * @brief: 初始化断点
 * @param {BreakPoint*} breakpoint 断点数组首地址
 * @param {char*} elf_path 可执行文件路径，用于解析符号地址
 * @param {int} pid
 * @description: 解析配置文件和ELF文件，获取所有需要设置断点的符号及对应地址，最后在符号的首条指令处设置断点
 * @return {int} 错误码，执行成功返回1，否则返回-1
 */
int breakpoint_initialize(struct BreakPoint* breakpoint, int count, char* elf_path, int pid);


/**
 * @brief: 单步越过断点
 * @param {BreakPoint*} breakpoint 断点数组首地址
 * @param {int} pid
 * @description: 跨越断点，避免无限循环
 * @return {int} 错误码，执行成功返回1，否则返回-1
 */
int step_over_breakpoint(struct BreakPoint* breakpoint, int count, int pid);


/**
 * @brief: 获取符号在ELF文件中的偏移量
 * @param {char*} file_path
 * @param {char*} target_symbol
 * @param {long*} symbol_offset 用于存放偏移量的变量地址
 * @description: 
 * @return {int} 错误码，为-1时表示出现错误
 */
int get_symbol_offset(char* file_path, char* target_symbol, long* symbol_offset);


/**
 * @brief: 获取被跟踪进程在虚拟内存中的加载地址
 * @param {int} pid 被跟踪进程PID
 * @description: 
 * @return {int}
 */
int get_load_address(int pid, long* load_address);


#endif