/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-07-24 21:45:55
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-13 13:00:29
 * @FilePath: /wc-intercept/wc-ptrace/src/elf.cc
 * @Description: ELF类实现
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include "elf.h"
#include "log.h"
#include "target.h"

namespace wc {

static Logger::ptr logger = WC_LOG_NAME("intercept");
    
TraceeElf::TraceeElf(const std::string& path) {
    // 检测路径有效性
    if (path.empty() || access(path.c_str(), F_OK) != 0) {
        std::logic_error("Error elf file path!");
        return;
    }
    m_path = path;

    // 打开并检查可执行文件
    if ( elf_version(EV_CURRENT) == EV_NONE ) {
        std::logic_error(elf_errmsg(-1));
        return;
    }

    if ( (m_fd=open(m_path.c_str(), O_RDONLY, 0)) < 0 ){
        std::logic_error("ELF path error, Can't open!");
        return;
    }

    if ( (m_object=elf_begin(m_fd, ELF_C_READ, NULL)) == NULL ){
        std::logic_error(elf_errmsg(-1));
        close(m_fd);
        elf_end(m_object);
        return;
    }
    
    if ( elf_kind(m_object) != ELF_K_ELF ){
        std::logic_error("Not a execute object!");
        close(m_fd);
        elf_end(m_object);
        return;
    }

    // 获取strtab section在Section Header中的索引
    if (elf_getshdrstrndx(m_object, &m_strndx) != 0){
        std::logic_error(elf_errmsg(-1));
        close(m_fd);
        elf_end(m_object);
        return;
    }

    m_rela_plt = nullptr;
    m_symtab = nullptr;
    m_dynsym = nullptr;
    m_dynamic = nullptr;
}

TraceeElf::~TraceeElf() {
    if ( m_object != nullptr ) {
        elf_end(m_object);
    }
    if (m_fd != -1) {
        close(m_fd);
    }
}

void TraceeElf::initialize() {
    Elf_Scn * scn = NULL;
    GElf_Shdr shdr;
    char * name;
    int index = 0;
    while ( (scn=elf_nextscn(m_object, scn)) != NULL ) {
        if ( gelf_getshdr(scn, &shdr) != &shdr ) {
            std::logic_error("gelf_getshdr() failed: "+std::string(elf_errmsg(-1)));
            return;
        }
        if ( (name=elf_strptr(m_object, m_strndx, shdr.sh_name)) == NULL ) {
            std::logic_error("gelf_getshdr() failed: "+std::string(elf_errmsg(-1)));
            return;
        }
        if ( strcmp(name, RELAPLT) == 0 ) {
            Elf_Data * data;
            if ((data = elf_getdata(scn, NULL)) == NULL) {
                std::logic_error("Can't get .rela.plt section Elf_Data: "+std::string(elf_errmsg(-1)));
                return;
            }
            m_rela_plt = Section::ptr(new Section(RELAPLT, data, scn, shdr, index));
        }
        if ( strcmp(name, SYMTAB) == 0 ) {
            Elf_Data * data;
            if ((data = elf_getdata(scn, NULL)) == NULL) {
                std::logic_error("Can't get .symtab section Elf_Data: "+std::string(elf_errmsg(-1)));
                return;
            }
            m_symtab = Section::ptr(new Section(SYMTAB, data, scn, shdr, index));
        }
        if ( strcmp(name, DYNSYM) == 0 ) {
            Elf_Data * data;
            if ((data = elf_getdata(scn, NULL)) == NULL) {
                std::logic_error("Can't get .dynsym section Elf_Data: "+std::string(elf_errmsg(-1)));
                return;
            }
            m_dynsym = Section::ptr(new Section(DYNSYM, data, scn, shdr, index));
        }
        if ( strcmp(name, DYNAMIC) == 0 ) {
            Elf_Data * data;
            if ((data = elf_getdata(scn, NULL)) == NULL) {
                std::logic_error("Can't get .dynamic section Elf_Data: "+std::string(elf_errmsg(-1)));
                return;
            }
            m_dynamic = Section::ptr(new Section(DYNAMIC, data, scn, shdr, index));
        }
        ++index;
    }

    /* 使用dymanic section检测是否延迟绑定，是则需要在第一次函数被调用后需要更新地址*/
    if (m_dynamic) {
        int bind_now = search_dynamic(DT_BIND_NOW);
        if (bind_now != -1) {
            m_bind_now = true;
        }
        else {
            bind_now = search_dynamic(DT_FLAGS);
            if (bind_now == -1) {
                m_bind_now = false;
            }
            else if (((GElf_Dyn*)(m_dynamic->m_data->d_buf)+bind_now)->d_un.d_val==DF_BIND_NOW) {
                m_bind_now = true;
            }
        }
    }

    /* 获取ELF Header */
    if (gelf_getehdr(m_object, &m_ehdr) != &m_ehdr) {
        std::logic_error("Can't get ELF Header: "+std::string(elf_errmsg(-1)));
        return;
    }
}

int TraceeElf::search_dynamic(int d_tag) {
    if (d_tag<0) return -1;

    if (m_dynamic->m_data->d_size > 0) {
        int count = m_dynamic->get_entry_size();
        for (int i=0; i<count; ++i) {
            GElf_Dyn dyn;
            if (gelf_getdyn(m_dynamic->m_data, i, &dyn) != &dyn) {
                WC_LOG_ERROR(logger) << "[TraceeElf::search_dynamic] Can't find sym in dynamic by entry index "
                                     << d_tag << " " << elf_errmsg(-1) << std::endl;
                return -1;
            }
            if (d_tag == dyn.d_tag) return i;
        }
    } 
    return -1;
}

int TraceeElf::search_symbol(const std::string& symtab_name, const std::string& symbol_name) {
    if (symtab_name.empty() || symbol_name.empty()) {
        return -1;
    }
    Section::ptr symtab = nullptr;
    if (symtab_name == DYNSYM) {
        symtab = m_dynsym;
    }
    else if (symtab_name == SYMTAB) {
        symtab = m_symtab;
    }
    if (symtab == nullptr) {
        return -1;
    }

    size_t strndx = symtab->get_strndx();
    if (symtab->m_data->d_size > 0) {
        int count = symtab->get_entry_size();
        for (int i=0; i<count; ++i) {
            GElf_Sym symbol;
            if (gelf_getsym(symtab->m_data, i, &symbol) != &symbol) {
                WC_LOG_ERROR(logger) << "[TraceeElf::search_symbol] Can't get GElf_Sym in search " << symbol_name
                                     << " " << elf_errmsg(-1) << std::endl;
                return -1;
            }
            char * name;
            if ((name=elf_strptr(m_object, strndx, symbol.st_name)) == NULL ) {
                WC_LOG_ERROR(logger) << "[TraceeElf::search_symbol]: elf_strptr() search symbol " << symbol_name
                                     << "failed " << elf_errmsg(-1) << std::endl;
                return -1;
            }
            if (symbol_name == std::string(name)) {
                WC_LOG_INFO(logger) << "[TraceeElf::search_symbol]: search symbol: " << symbol_name
                                     << "success, symbol index in " << symtab_name << "is " << i
                                     << elf_errmsg(-1) << std::endl;
                return i;
            }
        }
    } 
    return -1;
}

int TraceeElf::search_rela_plt(int index) {
    if (index < 1) {
        return -1;
    }
    size_t strndx = m_rela_plt->get_strndx();
    if (m_rela_plt->m_data->d_size > 0) {
        int count = m_rela_plt->get_entry_size();
        for (int i=0; i<count; ++i) {
            GElf_Rela rela;
            if (gelf_getrela(m_rela_plt->m_data, i, &rela) != &rela) {
                WC_LOG_ERROR(logger) << "[TraceeElf::search_rela_plt]: Can't get GElf_Rela " 
                                     << elf_errmsg(-1) << std::endl;
                return -1;
            }
            if (GELF_R_SYM(rela.r_info) == index) {
                WC_LOG_INFO(logger) << "[TraceeElf::search_rela_plt]: search rela index success, symbol in dynsym index " << index
                                     << ", index of rela is " << i << elf_errmsg(-1) << std::endl;
                return i;
            }
        }
    }
    return -1;
}

GElf_Sym* TraceeElf::get_sym_in_dynsym(int index) {
    return ((GElf_Sym*)(m_dynsym->m_data->d_buf)+index);
}

GElf_Sym* TraceeElf::get_sym_in_symtab(int index) {
    return ((GElf_Sym*)(m_symtab->m_data->d_buf)+index);
}

GElf_Sym *TraceeElf::get_sym(std::string& table_name, int index) {
    if (table_name==DYNSYM) {
        return ((GElf_Sym*)(m_dynsym->m_data->d_buf)+index);
    }
    else {
        return ((GElf_Sym*)(m_symtab->m_data->d_buf)+index);
    }
}

GElf_Rela* TraceeElf::get_sym_in_relaplt(int index) {
    return ((GElf_Rela*)(m_rela_plt->m_data->d_buf)+index);
}

bool TraceeElf::is_function(const uint8_t& info) {
    if (info == SHN_UNDEF || info == SHN_ABS || info == SHN_COMMON) {
        return false;
    }
    if (ELF64_ST_TYPE(info) == STT_FUNC) {
        return true;
    }
    return false;
}

int get_load_address(int pid, long* load_address) {
    
    /* 打开/proc/pid/maps文件 */
    char proc_file[64] = "";
    sprintf(proc_file, "/proc/%d/maps", pid);
    FILE* proc_fp = fopen(proc_file, "r");
    if (proc_file == NULL) {
        WC_LOG_ERROR(logger) << "[get_load_address]: Can't open proc file " << proc_file << std::endl;
        return -1;
    }

    char line[256] = "";
    char load_address_str[40] = "";
    char *offest_address = NULL;
    long load_address_int = 0;
    fgets(line, 256, proc_fp);

    /* 从proc文件中获取进程加载地址 */
    if ((offest_address = strchr(line, '-')) == NULL) {
        WC_LOG_ERROR(logger) << "[get_load_address]: Can't match load address " << proc_file << std::endl;
        return -1;
    }
    strncpy(load_address_str, line, offest_address-line);   
    sscanf(load_address_str, "%lx", &load_address_int); // Converts a string to hex
    
    *load_address = load_address_int;
    return 1;
}

}