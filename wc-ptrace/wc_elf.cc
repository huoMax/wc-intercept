/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-07-13 23:19:06
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-07-14 00:43:40
 * @FilePath: /wc-intercept/wc-ptrace/wc_elf.cc
 * @Description: 封装elf相关接口实现
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include "wc_elf.h"

namespace wc_elf {

TraceeElf::TraceeElf(const std::string& path) {
    m_elf_path = path;
    m_fd = -1;
    m_elf_object = nullptr;
    m_strtab_index = -1;

    m_rela_plt_section = nullptr;
    m_got_section = nullptr;
    m_symtab_section = nullptr;
}

TraceeElf::~TraceeElf() {

    if ( m_elf_object != nullptr ) {
        elf_end(m_elf_object);
    }

    if (m_fd != -1) {
        close(m_fd);
    }
}

void TraceeElf::initialize() {

    // 打开并检查可执行文件
    if ( elf_version(EV_CURRENT) == EV_NONE )
        errx(EXIT_FAILURE, "ELF library initialization failed: %s", elf_errmsg(-1));

    if ( (m_fd=open(m_elf_path.c_str(), O_RDONLY, 0)) < 0 )
        errx(EXIT_FAILURE, "open %s failed", m_elf_path.c_str());

    if ( (m_elf_object=elf_begin(m_fd, ELF_C_READ, NULL)) == NULL )
        errx(EXIT_FAILURE, "elf_begin() failed: %s", elf_errmsg(-1));
    
    Elf_Kind ek = elf_kind(m_elf_object);
    if ( ek != ELF_K_ELF )
        errx(EXIT_FAILURE, "%s not a elf object: %s", m_elf_path.c_str(), elf_errmsg(-1));

    // 获取strtab section
    if (elf_getshdrstrndx(m_elf_object, &m_strtab_index) != 0)
        errx(EXIT_FAILURE, "elf_getshdrstrndx() failed: %s.", elf_errmsg(-1));
        
    // 获取symtab section、rela.plt section、got section
    Elf_Scn * scn = NULL;
    GElf_Shdr shdr;
    char * name;
    while ( (scn=elf_nextscn(m_elf_object, scn)) != NULL ) {

        if ( gelf_getshdr(scn, &shdr) != &shdr )
            errx(EXIT_FAILURE, "getshdr() failed: %s.", elf_errmsg(-1));
        
        if ( (name=elf_strptr(m_elf_object, m_strtab_index, shdr.sh_name)) == NULL )
            errx(EXIT_FAILURE, "elf_strptr() failed: %s.", elf_errmsg(-1));
        
        if ( strcmp(name, ".got") == 0 ) {
            m_got_section = scn;
        }
        if ( strcmp(name, ".rela.plt") == 0 ) {
            m_rela_plt_section = scn;
        }
        if ( strcmp(name, ".symtab" ) == 0 ) {
            m_symtab_section = scn;
        }

        printf("Section %s\n", name);
    }

    // 读取配置文件、初始化要拦截的函数符号相关数据结构

    // 初始化断点
}



}

