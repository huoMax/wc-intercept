/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-07-11 23:36:11
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-07-12 00:16:57
 * @FilePath: /wc-intercept/wc-ptrace/learning/libelf-example/get_elf_header.cc
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

/* libelf library include - GELF(3) */
#include <gelf.h>

int main(int argc, char **argv) {
    
    int i, fd;
    Elf *e;
    char *id, bytes[5];
    size_t n;

    /*
     * 定义于/usr/include/gelf.h，实际上就是我们之前所说的glibc定义的ELF头
     * typedef Elf64_Ehdr GElf_Ehdr;
     * GELF也是libelf的一部分，它在ELF文件的本地副本上操作，它可以处理一些内存对齐之类的问题
    */
    GElf_Ehdr ehdr;

    if (argc != 2) 
        errx(EXIT_FAILURE, "usage: %s file-name", argv[0]);
    
    if ( elf_version(EV_CURRENT) == EV_NONE )
        errx (EXIT_FAILURE, "ELF library initialization failed : %s" , elf_errmsg(-1));

    if ((fd = open(argv[1], O_RDONLY, 0)) < 0)
        errx(EXIT_FAILURE, "open: %s failed", argv[1]);
    
    if ((e = elf_begin(fd, ELF_C_READ, NULL)) == NULL)
        errx(EXIT_FAILURE, "elf_begin failed: %s", elf_errmsg(-1));
    
    if (elf_kind(e) != ELF_K_ELF)
        errx(EXIT_FAILURE, "%s is not an ELF object.", argv[1]);

    /*
     * gelf_getehdr用于读取ELF Header
     * 它将ELF对象的Header转为GElf_Ehdr的形式
     */
    if (gelf_getehdr(e, &ehdr) == NULL) {
        errx(EXIT_FAILURE, "getehdr failed: %s", elf_errmsg(-1));
    }

    /* 
     * 检索ELF的class
     */
    if ((i=gelf_getclass(e)) == ELFCLASSNONE)
        errx(EXIT_FAILURE, "getclass failed: %s", elf_errmsg(-1));

    printf("%s: %d-bit ELF object\n", argv[1], i == ELFCLASS32 ? 32 : 64);

    /*
     * 获取ELF的魔数
    */
    if ((id = elf_getident(e, NULL)) == NULL)
        errx(EXIT_FAILURE, "getident failed: %s", elf_errmsg(-1));
    
    printf("%3s e_ident[0..%1d] %7s", " ", EI_ABIVERSION, " ");

    printf ( "\n " );

    # define PRINT_FMT "%-20s 0x%lx\n"
    # define PRINT_FIELD(N) do { \
        printf (PRINT_FMT, #N, (uintmax_t)ehdr.N); \
    } while(0)

    /*
     * 我们已经知道GElf_Ehdr其实就是glibc定义的Elf64_Ehdr，我们已经通过gelf_getehdr()获取的了它
     * 可以按照结构体正常方式输出其中的值
     */
    PRINT_FIELD(e_type);
    PRINT_FIELD(e_machine);
    PRINT_FIELD(e_version);
    PRINT_FIELD(e_entry);
    PRINT_FIELD(e_phoff);
    PRINT_FIELD(e_shoff);
    PRINT_FIELD(e_flags);
    PRINT_FIELD(e_ehsize);
    PRINT_FIELD(e_phentsize);
    PRINT_FIELD(e_shentsize);

    /*
     * 获取Section Header Table的的表项数量
     */
    if ( elf_getshdrnum (e, &n ) != 0)
        errx ( EXIT_FAILURE , "getshdrnum() failed: %s. " , elf_errmsg ( -1));
    printf ( PRINT_FMT , " ( shnum ) " , ( uintmax_t )n );

    /*
     * 获取.strtab的的表项数量
     */
    if ( elf_getshdrstrndx (e, &n ) != 0)
        errx ( EXIT_FAILURE , " getshdrstrndx() failed : %s." , elf_errmsg ( -1));
    printf ( PRINT_FMT , " ( shstrndx )" , ( uintmax_t )n );

    /*
     * 获取Program Header Table的的表项数量
     */
    if ( elf_getphdrnum (e , &n ) != 0)
        errx ( EXIT_FAILURE , " getphdrnum() failed: %s ." ,
        elf_errmsg ( -1));
    printf ( PRINT_FMT , " ( phnum ) " , (uintmax_t)n );

    elf_end ( e );
    close ( fd );
    exit ( EXIT_SUCCESS );

}