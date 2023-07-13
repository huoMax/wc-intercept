/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-07-12 04:50:31
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-07-14 00:41:41
 * @FilePath: /wc-intercept/wc-ptrace/wc_elf.h
 * @Description: 封装elf相关接口
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <string.h>

#include <string>
#include <memory>
#include <vector>
#include <map>

/* libelf */
#include <elf.h>
#include <gelf.h>

#define FUNCTION_START 0x1
#define FUNCTION_RETURN 0x2

namespace wc_elf {

struct break_point {
    bool st_enable;                    // 是否设置断点
    int8_t st_flag;                    // 断点类型
    uint64_t st_instruction_content;   // 被插入的指令内容
    uint64_t st_instruction_address;   // 被插入断点的指令地址
};

struct symbol {
    
    /* 符号基本信息 */
    std::string st_name;        // 符号名
    uint8_t st_bind;            // 符号绑定信息
    uint8_t st_type;            // 符号类型
    uint16_t st_ndx;            // 符号所在节索引
    uint64_t st_value;          // 符号值

    /* 断点相关信息 */
    struct break_point st_start_bp;     // 符号所关联的首地址断点
    struct break_point st_return_bp;    // 符号所关联的返回断点

    /* 重定位相关信息 */
    bool st_rela;               // 是否需要重定位
    bool st_db;                 // 是否设置了延迟绑定
    bool st_already;            // 是否完成了延迟绑定
};


class TraceeElf {
public:

    /**
     * @brief: 构造函数
     */    
    TraceeElf(const std::string& path);

    /**
     * @brief: 析构函数
     */    
    ~TraceeElf();

    /**
     * @brief: 初始化ELF类
     */    
    void initialize();

    // /**
    //  * @brief: 添加要拦截的函数符号
    //  * @param {string&} symbol_name
    //  */    
    // void add_symbol(const std::string& symbol_name);

    // /**
    //  * @brief: 删除要拦截的函数符号
    //  * @param {string&} symbol_name
    //  */    
    // void delete_symbol(const std::string& symbol_name);

    // /**
    //  * @brief: 更新延迟绑定的函数符号地址
    //  * @param {string&} symbol_name
    //  */    
    // void update_symbol(const std::string& symbol_name);

    // /**
    //  * @brief: 初始化函数符号
    //  */    
    // void initialize_symbols();

    // /**
    //  * @brief: 初始化断点
    //  */
    // void initialize_bp();
    
private:
    std::string m_elf_path;
    int m_fd;
    Elf * m_elf_object;                     // ELF对象指针
    size_t m_strtab_index;                  // strtab section在Section Header中的索引

    Elf_Scn * m_rela_plt_section;               // 重定位section指针
    Elf_Scn * m_got_section;                // 全局数据section指针
    Elf_Scn * m_symtab_section;             // symtab指针

    std::vector<struct symbol> m_symbols;   // 需要拦截的函数数组
    std::map<uint64_t, uint32_t> m_indexs;  // 断点地址对应的函数在函数数组中的索引
};


// // 错误码
// static int wc_elf_errno = -1;

// /**
//  * @brief: 打开ELF对象，返回Elf对象指针
//  * @param {string} elf_file_path : ELF文件路径
//  * @param {Elf *} elf_object: 用于获取打开的elf对象指针
//  */
// Elf * 
// initiailize (std::string& elf_file_path, Elf& elf_object);

// /**
//  * @brief: 关闭打开的Elf对象，释放对应的空间
//  * @param {Elf *} elf_object : Elf对象指针
//  */
// void
// elf_close(Elf& elf_object);

// /**
//  * @brief: 返回Section Header Table中strtab的索引
//  * @param {Elf *} elf_object: Elf对象指针
//  * @param {size_t *} index: 用来保存返回的索引
//  */
// size_t
// get_strtab_index(Elf& elf_object, size_t& index);

// /**
//  * @brief: 返回strtab中指定索引的字符串
//  * @param {Elf *} elf_object: Elf对象指针
//  * @param {size_t&} strtab_index: strtab在Section Header Table中的索引
//  * @param {size_t&} str_offset: 字符串在strtab中的偏移量
//  */
// std::string&
// get_strname(Elf& elf_object, const size_t& strtab_index, const size_t& str_offset);

// /**
//  * @brief: 从Section Header Table中获取section的描述信息
//  * @param {Elf_Scn&} section_scn：section的Elf_Scn，指向section
//  * @param {GElf_Shdr&} section_header
//  */
// GElf_Shdr&
// get_sction_header(const Elf_Scn& section_scn, GElf_Shdr& section_header);

// /**
//  * @brief: 获取指定的section，返回Elf_Scn对象
//  * @param {Elf&} elf_object
//  * @param {string} section_name
//  */
// Elf_Scn&
// get_section(Elf& elf_object, std::string section_name);

}