/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-07-24 20:31:02
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-13 12:59:04
 * @FilePath: /wc-intercept/wc-ptrace/src/elf.h
 * @Description: ELF文件的抽象类，封装相应的函数
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#ifndef __WC_INTERCPET_ELF_H__
#define __WC_INTERCPET_ELF_H__

/* C Library */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/user.h> 
#include <sys/wait.h>

/* C++ Library */
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <functional>

/* libelf */
#include <elf.h>
#include <gelf.h>

#include "target.h"

#define SYMTAB  ".symtab"
#define DYNSYM  ".dynsym"
#define RELAPLT ".rela.plt"
#define DYNAMIC ".dynamic"

namespace wc {
/**
 * @brief: ELF section类
 */
class Section {
public:
    // 智能指针类型内嵌
    typedef std::shared_ptr<Section> ptr;

    /**
     * @brief: 构造函数
     * @param {string&} name
     * @param {Elf_Data*} data
     * @param {Elf_Scn*} scn
     * @param {GElf_Shdr&} shdr
     * @param {int} index
     */    
    Section(const std::string& name, Elf_Data* data, Elf_Scn* scn, GElf_Shdr& shdr, int index)
        : m_name(name)
        , m_data(data)
        , m_scn(scn)
        , m_shdr(shdr)
        , m_index(index) {}

    /**
     * @brief: 返回section对应的strtab的索引，用于表项名称的搜索
     */    
    int get_strndx() { return m_shdr.sh_link; }

    /**
     * @brief: 返回表项数量
     */    
    int get_entry_size() {
        if (m_shdr.sh_entsize) {
            return m_shdr.sh_size / m_shdr.sh_entsize;
        }
        return 0;
    }
    
    std::string m_name;         // section name
    Elf_Data * m_data;          // section data ptr
    Elf_Scn * m_scn;            // section scn ptr
    GElf_Shdr m_shdr;           // section对应Section Header中的表项
    int m_index;                // 在sectinos中的索引（可能有用）
};

/**
 * @brief: ELF Object类
 */
class TraceeElf {
public:
    // 智能指针类型
    typedef std::shared_ptr<TraceeElf> ptr;
    
    /**
     * @brief: 构造函数，打开ELF文件
     */    
    TraceeElf(const std::string& path);

    /**
     * @brief: 析构函数
     */    
    ~TraceeElf();

    /**
     * @brief: 初始化与本项目相关的section类
     * @details: .rela.plt、.symtab、.dynsym、.dynamic
     */    
    void initialize();

    /**
     * @brief: 从ELF文件中查找指定函数符号，并返回初始化后的Func
     * @param {string&} symbol_name
     * @return {std::shared_ptr<Func>} 成功返回构造好的Func共享指针，失败返回nullptr
     */    
    Function::ptr get_func(const std::string& symbol_name);

    /**
     * @brief: 该ELF文件是否是立即绑定
     */    
    bool is_bind_now() { return m_bind_now; }

    /**
     * @brief: 获取入口地址
     */    
    uint64_t get_entry() { return m_ehdr.e_entry; }

    /**
     * @brief: 获取ELF文件路径
     */    
    std::string get_name() const { return m_path; }

public:
    /**
     * @brief: 在动态链接表.dynamic中查找指定类型的表项，返回表项在.dynamic中的索引
     * @param {int} d_tag 表项类型, 通过查询glibc中elf.h可知
     * @return {int} 搜索成功则返回符号在表中的索引，否则返回-1
     */    
    int search_dynamic(int d_tag);

    /**
     * @brief: 在指定符号表中搜索符号，返回符号在符号表中的索引, 支持.dynsym、.symtab
     * @details: glibc中的符号在.dynsym中为函数名，在.symtab中为函数名@glibc_version
     * @param {const std::string&} symtab_name
     * @param {string&} symbol_name
     * @return {int} 搜索成功则返回符号在符号表中的索引，否则返回-1
     */    
    int search_symbol(const std::string& symtab_name, const std::string& symbol_name);

    /**
     * @brief: 在重定位表.rela.plt中搜索.dynsym中对应的重定位项，返回在.rela.plt中的索引
     * @param {int} index
     */    
    int search_rela_plt(int index);

public:
    /**
     * @brief: 通过符号在.dynsym中的索引获取对应的GElf_Sym指针
     * @param {int} index
     */    
    GElf_Sym* get_sym_in_dynsym(int index);

    /**
     * @brief: 通过符号在.symtab中的索引获取对应的GElf_Sym指针
     * @param {int} index
     */    
    GElf_Sym* get_sym_in_symtab(int index);

    /**
     * @brief: 根据符号的索引获取其在符号表table_name中的表项指针
     * @param {string&} table_name
     * @param {int} index
     */    
    GElf_Sym *get_sym(std::string& table_name, int index);

    /**
     * @brief: 通过符号在.rela.plt中的索引获取对应的GElf_Rela指针
     * @param {int} index
     */ 
    GElf_Rela* get_sym_in_relaplt(int index);

public:
    /**
     * @brief: 通过符号的m_info字段检查符号是否是函数
     * @param {uint8_t&} info
     */    
    static bool is_function(const uint8_t& info);

private:
    std::string m_path;                     // ELF 文件路径
    int m_fd;                               // 打开的ELF文件句柄
    Elf * m_object;                         // ELF对象指针
    GElf_Ehdr m_ehdr;                     // ELF Header
    size_t m_strndx;                        // strtab section在Section Header中的索引

    bool m_bind_now;                        // 是否使用了延迟绑定
    std::shared_ptr<Section> m_rela_plt;    // .rela.plt
    std::shared_ptr<Section> m_symtab;      // .symtab
    std::shared_ptr<Section> m_dynsym;      // .dynsym
    std::shared_ptr<Section> m_dynamic;     // .dynamic
};

int get_load_address(int pid, long* load_address);

} // namespace wc

#endif