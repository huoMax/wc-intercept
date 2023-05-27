/*
 * @Author: huomax
 * @Date: 2023-05-24 23:43:06
 * @LastEditors: huomax
 * @LastEditTime: 2023-05-27 22:59:26
 * @FilePath: /wgk/wc-intercept/ptrace/src/wc_elf.c
 * @Description: 解析被跟踪进程的ELF文件
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */

#include "wc_include.h"
#include "wc_elf.h"

long RunTo(int pid, int status, long address) {

    int flag = 0;
    long current_rbp = 0;
    long ret_address = 0;

    /* 运行直到下一条指令为指定符号首地址 */
    if(WIFSTOPPED(status)) {
        while(1) {
            ptrace(PTRACE_SINGLESTEP, pid, NULL, NULL);
            waitpid(pid, &status, 0);
            if(WIFEXITED(status)) {
                printf("[RunTo]: tracee exited!\n");
                return 0;
            }
            struct user_regs_struct regs;
            if(WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
                ptrace(PTRACE_GETREGS, pid, NULL, &regs);
                if (current_rbp != (long)regs.rbp && flag == 1) {
                    ret_address = ptrace(PTRACE_PEEKDATA, pid, (long)regs.rbp + sizeof(long), NULL);
                    break;
                }
                if (regs.rip == address) {
                    current_rbp = (long)regs.rbp;
                    flag = 1;
                }
            }
        }
    }
    else {
        printf("[RunTo]: tracee status error!\n");
        exit(-1);
    }

    return ret_address;
}

long GetSymbolAddress(char* file_path, char* target_symbol) {

    /* 初始化ELF文件和对象 */
    Elf * elf;
    FILE * fp = NULL;
    if (elf_version(EV_CURRENT) == EV_NONE) {
        fprintf(stderr, "[get_symbol_address]: ELF library initialization failed: %s\n", elf_errmsg(-1));
        exit(1);
    }
    if ((fp = fopen(file_path, "r")) == NULL) {
        fprintf(stderr, "[get_symbol_address]: Failed to open ELF file\n");
        exit(2);
    }
    if ((elf = elf_begin(fileno(fp), ELF_C_READ, NULL)) == NULL) {
        fprintf(stderr, "[get_symbol_address]: Failed to get ELF object with error: %s.\n", elf_errmsg(elf_errno()));
        exit(2);
    }
    if (elf_kind(elf) != ELF_K_ELF) {
        fprintf(stderr, "[get_symbol_address]: Not an ELF file\n");
        elf_end(elf);
        exit(3);
    }


    /* 遍历sectioin找到symtab，从symtab中获取指定符号的地址 */
    Elf_Scn * scn = NULL; 
    while ((scn = elf_nextscn(elf, scn)) != NULL) {
        GElf_Shdr shdr;
        if (gelf_getshdr(scn, &shdr) != &shdr) {
            fprintf(stderr, "[get_symbol_address]: Failed to get section header: %s.\n", elf_errmsg(elf_errno()));
            exit(4);
        }
        if (shdr.sh_type == SHT_SYMTAB || shdr.sh_type == SHT_DYNSYM) {
            Elf_Data * data;
            if ((data = elf_getdata(scn, NULL)) == NULL) {
                fprintf(stderr, "[get_symbol_address]: Failed to get section data: %s.\n", elf_errmsg(elf_errno()));
                elf_end(elf);
                exit(5);
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

                if (strcmp(symname, target_symbol) == 0) {
                        elf_end(elf);
                        fclose(fp);
                        return addr;
                    }
            }
        }
    }

    elf_end(elf);
    fclose(fp);
    return 0;
}

long GetEntryAddress(char* file_path) {
    int fd;
    Elf *elf;
    GElf_Ehdr ehdr;
    GElf_Addr entry;

    if ((fd = open(file_path, O_RDONLY, 0)) < 0) 
        FATAL("[get_entry_address]: cant't open %s <binary>\n", file_path);

    if (elf_version(EV_CURRENT) == EV_NONE) 
        FATAL("[get_entry_address]: ELF library initialization failure\n");

    if ((elf = elf_begin(fd, ELF_C_READ, NULL)) == NULL) 
        FATAL("[get_entry_address]: get elf_begin failure!\n");
    
    if (gelf_getehdr(elf, &ehdr) == NULL)
        FATAL("[get_entry_address]: get gelf_getehdr failure!\n");
    
    entry = ehdr.e_entry;

    elf_end(elf);
    close(fd);

    return entry;
}

long GetLoadAddress(int pid) {
    
    /* 打开/proc/pid/maps文件 */
    char proc_file[30] = "";
    sprintf(proc_file, "/proc/%d/maps", pid);
    FILE* proc_fp = fopen(proc_file, "r");
    if (proc_file == NULL) {
        FATAL("read proc file failure! [%s]", strerror(errno));
    }

    char line[256] = "";
    char load_address_str[40] = "";
    char *offest_address = NULL;
    long int load_address_int = 0;
    fgets(line, 256, proc_fp);

    
    /* 从proc文件中获取进程加载地址 */
    if ((offest_address = strchr(line, '-')) == NULL) {
        FATAL("match load address failure! [%s]", strerror(errno));
    }
    strncpy(load_address_str, line, offest_address-line);   
    sscanf(load_address_str, "%lx", &load_address_int); // Converts a string to hex
    printf("load_address_int: %lx\n", load_address_int);

    return load_address_int;
}