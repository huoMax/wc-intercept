/*
 * @Author: huomax
 * @Date: 2023-06-05 06:40:47
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-07-24 20:27:42
 * @FilePath: /wc-intercept/ptrace/src/wc_parser.c
 * @Description: 断点相关函数实现
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */

#include "wc_parser.h"

struct BreakPoint* parser_config(char* config_path, int* ret_count) {

    FILE *fp = NULL;
    fp = fopen(config_path, "r");
    if (fp == NULL) {
        fprintf(stderr, "[parser_config]: open config file failure, please check config file!\n");
        return NULL;
    }

    struct BreakPoint* breakpoint_array = NULL;
    char line[MAX_READ_LINE] = "";
    char functions[MAX_READ_LINE] = "";
    int count = 0;
    while (fgets(line, MAX_READ_LINE, fp) != NULL) {
        if (strncmp(line, "[breakpoint]", 12) == 0){
            memset(line, 0, sizeof(line));
            while (fgets(line, MAX_READ_LINE, fp) != NULL) {
                if (strncmp(line, "function", 8) == 0) {
                    char *split = strtok(line+8, " ");
                    while(split != NULL) {
                        int start = 0;
                        if (strcmp(split, "=") == 0) {
                            split = strtok(NULL, " ");
                            continue;
                        }
                        else if (split[0] == '=' && strlen(split) > 1) {
                            start = 1;
                        }
                        ++count;
                        strcat(functions, split+start);
                        strcat(functions, " ");
                        split = strtok(NULL, " ");
                    }
                    break;
                }
                memset(line, 0, sizeof(line));
            }
            break;
        }
        memset(line, 0, sizeof(line));
    }

    if (count==0) {
        return NULL;
    }
    else {
        breakpoint_array = (struct BreakPoint*)malloc(count * sizeof(struct BreakPoint));
        memset(breakpoint_array, 0, sizeof(breakpoint_array));
        int brk = 0;
        char *split = strtok(functions, " ");
        while (split != NULL) {
            strcpy((breakpoint_array+brk)->symbol, split);
            ++brk;
            split = strtok(NULL, " ");
        }
    }

    *ret_count = count;
    fclose(fp);
    return breakpoint_array;
}


int breakpoint_initialize(struct BreakPoint* breakpoint, int count, char* elf_path, int pid) {

    long load_address = 0;
    if (get_load_address(pid, &load_address) == -1) {
        return -1;
    }
    for(int idx=0; idx<count; ++idx) {
        long symbol_offset = 0;
        if (get_symbol_offset(elf_path, (breakpoint+idx)->symbol, &symbol_offset) == -1) {
            return -1;
        }
        (breakpoint+idx)->head.address = symbol_offset + load_address;
    }

    for (int idx=0; idx<count; ++idx) {
        if (breakpoint_enable(&((breakpoint+idx)->head), (breakpoint+idx)->symbol, pid) == -1) {
            return -1;
        }
    }
    return 1;
}

int breakpoint_enable(struct Record* record, char * symbol, int pid) {

    long data = ptrace(PTRACE_PEEKDATA, pid, record->address, NULL);
    if (data == -1) {
        fprintf(stderr, "[breakpoint_enable]: Get symbol: %s instruction content failed! erron: %s", symbol, strerror(errno));
        return -1;
    }
    record->save_data = (uint8_t)(data & 0xff);

    uint64_t data_with_int3 = ((data & ~0xff) | 0xcc);
    if (ptrace(PTRACE_POKEDATA, pid, record->address, data_with_int3) == -1){
        fprintf(stderr, "[breakpoint_enable]: Insert symbol: %s instruction content failed! erron: %s", symbol, strerror(errno));
        return -1;
    }
    record->enable = 1;
    return 1;
}

int breakpoint_disable(struct Record* record, char* symbol, int pid) {

    if (record->enable == 0) {
        fprintf(stderr, "[breakpoint_disable]: This instruction not set breakpoint!\n");
        return -1;
    }

    long data = ptrace(PTRACE_PEEKDATA, pid, record->address, NULL);
    if (data == -1) {
        fprintf(stderr, "[breakpoint_disable]: Get symbol: %s instruction content failed! erron: %s", symbol, strerror(errno));
        return -1;
    }

    uint64_t restored_data = ((data & ~0xff) | record->save_data);
    if (ptrace(PTRACE_POKEDATA, pid, record->address, restored_data) == -1) {
        fprintf(stderr, "[breakpoint_disable]: Restored symbol: %s instruction failed! erron: %s", symbol, strerror(errno));
        return -1;
    }
    record->enable = 0;
    return 1;
}

int step_over_breakpoint(struct BreakPoint* breakpoint_array, int count, int pid) {
    // 判断该断点是属于哪个函数、是首地址断点还是返回地址断点
    int status = 0;
    struct user_regs_struct regs;

    if (ptrace(PTRACE_GETREGS, pid, 0, &regs) == -1) {
        fprintf(stderr, "[step_over_breakpoint]: Get regs failed! erron: %s\n", strerror(errno));
        return -1;
    }
    uint64_t breakpoint_address = regs.rip - 1;
    int is_head = 1;
    struct BreakPoint * current_breakpoint = NULL;

    for (int idx=0; idx<count; ++idx) {

        if (breakpoint_address == (breakpoint_array+idx)->head.address) {
            current_breakpoint = (breakpoint_array+idx);
        }
        else if (breakpoint_address == (breakpoint_array+idx)->ret.address) {
            current_breakpoint = (breakpoint_array+idx);
            is_head = -1;
        }
    }

    // 遇到符号首地址断点需要重置返回地址断点
    if (is_head == 1) {
        long ret_address = ptrace(PTRACE_PEEKDATA, pid, regs.rsp, NULL);
        if (ret_address == -1) {
            fprintf(stderr, "[step_over_breakpoint]: Get return address failed! erron: %s\n", strerror(errno));
            return -1;
        }
        current_breakpoint->ret.address = ret_address;

        breakpoint_enable(&(current_breakpoint->ret), current_breakpoint->symbol, pid);
    }

    // 重置IR寄存器，因为遇到断点PC寄存器依旧加一了
    regs.rip = breakpoint_address;
    if (ptrace(PTRACE_SETREGS, pid, 0, &regs) == -1) {
        fprintf(stderr, "[step_over_breakpoint]: Reset IR register failed! erron: %s", strerror(errno));
        return -1;
    }

    struct Record * record = is_head == 1 ? &(current_breakpoint->head) : &(current_breakpoint->ret);
    breakpoint_disable(record, current_breakpoint->symbol, pid);
    if (ptrace(PTRACE_SINGLESTEP, pid, 0, 0) == -1) {
        fprintf(stderr, "[step_over_breakpoint]: Single step over breakpoint failed! erron: %s\n", strerror(errno));
        return -1;
    }
    waitpid(pid, &status, 0);

    // 符号返回地址处断点不需要重新设置，因为每次函数执行返回地址不同
    if (is_head == 1) {
        breakpoint_enable(record, current_breakpoint->symbol, pid);
    }
    return 1;
}

int get_symbol_offset(char* file_path, char* target_symbol, long* symbol_offset) {

    /* 初始化ELF文件和对象 */
    Elf * elf;
    FILE * fp = NULL;
    if (elf_version(EV_CURRENT) == EV_NONE) {
        fprintf(stderr, "[get_symbol_address]: ELF library initialization failed: %s\n", elf_errmsg(-1));
        return -1;
    }
    if ((fp = fopen(file_path, "r")) == NULL) {
        fprintf(stderr, "[get_symbol_address]: Failed to open ELF file\n");
        return -1;
    }
    if ((elf = elf_begin(fileno(fp), ELF_C_READ, NULL)) == NULL) {
        fprintf(stderr, "[get_symbol_address]: Failed to get ELF object with error: %s.\n", elf_errmsg(elf_errno()));
        fclose(fp);
        return -1;
    }
    if (elf_kind(elf) != ELF_K_ELF) {
        fprintf(stderr, "[get_symbol_address]: Not an ELF file\n");
        elf_end(elf);
        fclose(fp);
        return -1;
    }


    /* 遍历sectioin找到symtab，从symtab中获取指定符号的地址 */
    Elf_Scn * scn = NULL; 
    while ((scn = elf_nextscn(elf, scn)) != NULL) {
        GElf_Shdr shdr;
        if (gelf_getshdr(scn, &shdr) != &shdr) {
            fprintf(stderr, "[get_symbol_address]: Failed to get section header: %s.\n", elf_errmsg(elf_errno()));
            elf_end(elf);
            fclose(fp);
            return -1;
        }
        if (shdr.sh_type == SHT_SYMTAB || shdr.sh_type == SHT_DYNSYM) {
            Elf_Data * data;
            if ((data = elf_getdata(scn, NULL)) == NULL) {
                fprintf(stderr, "[get_symbol_address]: Failed to get section data: %s.\n", elf_errmsg(elf_errno()));
                elf_end(elf);
                fclose(fp);
                return -1;
            }

            int count = shdr.sh_size / shdr.sh_entsize;
            size_t stridx = shdr.sh_link;

            // 遍历符号表中的每个符号
            for (int i = 0; i < count; i++) {
                GElf_Sym sym;
                if (gelf_getsym(data, i, &sym) != &sym) {
                    fprintf(stderr, "[get_symbol_address]: Failed to get symbol: %s.\n", elf_errmsg(elf_errno()));
                    exit(6);
                }

                const char * symname = elf_strptr(elf, stridx, sym.st_name);
                Elf64_Addr addr = sym.st_value;

                if (strcmp(symname, target_symbol) == 0  ) {
                    *symbol_offset = addr;
                    elf_end(elf);
                    fclose(fp);
                    return 1;
                    }
            }
        }
    }

    elf_end(elf);
    fclose(fp);
    return -1;
}

int get_load_address(int pid, long* load_address) {
    
    /* 打开/proc/pid/maps文件 */
    char proc_file[64] = "";
    sprintf(proc_file, "/proc/%d/maps", pid);
    FILE* proc_fp = fopen(proc_file, "r");
    if (proc_file == NULL) {
        fprintf(stderr, "[get_load_address]: read proc file failure! [%s]", strerror(errno));
        return -1;
    }

    char line[256] = "";
    char load_address_str[40] = "";
    char *offest_address = NULL;
    long load_address_int = 0;
    fgets(line, 256, proc_fp);

    /* 从proc文件中获取进程加载地址 */
    if ((offest_address = strchr(line, '-')) == NULL) {
        fprintf(stderr, "match load address failure! [%s]", strerror(errno));
        return -1;
    }
    strncpy(load_address_str, line, offest_address-line);   
    sscanf(load_address_str, "%lx", &load_address_int); // Converts a string to hex
    
    *load_address = load_address_int;
    return 1;
}

