/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-07-12 00:49:22
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-07-12 00:51:43
 * @FilePath: /wc-intercept/wc-ptrace/learning/libelf-example/get_section_name.cc
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <err.h>
#include <fcntl.h>
#include <gelf.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
    int fd;
    Elf *e;
    char *name, *p, pc[4*sizeof(char)];
    Elf_Scn *scn;
    Elf_Data *data;
    GElf_Shdr shdr;
    size_t n, shstrndx, sz;

    if (argc != 2)
        errx(EXIT_FAILURE, "usage: %s file-name", argv[0]);

    if (elf_version(EV_CURRENT) == EV_NONE)
        errx(EXIT_FAILURE, "ELF library initialization "
            "failed: %s", elf_errmsg(-1));

    if ((fd = open(argv[1], O_RDONLY, 0)) < 0)
        err(EXIT_FAILURE, "open \%s\" failed", argv[1]);

    if ((e = elf_begin(fd, ELF_C_READ, NULL)) == NULL)
        errx(EXIT_FAILURE, "elf_begin() failed: %s.",
            elf_errmsg(-1));

    if (elf_kind(e) != ELF_K_ELF)
        errx(EXIT_FAILURE, "%s is not an ELF object.",
            argv[1]);

    if (elf_getshdrstrndx(e, &shstrndx) != 0)
        errx(EXIT_FAILURE, "elf_getshdrstrndx() failed: %s.",
            elf_errmsg(-1));

    scn = NULL;
    while ((scn = elf_nextscn(e, scn)) != NULL) { 
        if (gelf_getshdr(scn, &shdr) != &shdr) 
            errx(EXIT_FAILURE, "getshdr() failed: %s.",
                elf_errmsg(-1));

        if ((name = elf_strptr(e, shstrndx, shdr.sh_name))
            == NULL)
            errx(EXIT_FAILURE, "elf_strptr() failed: %s.",
                elf_errmsg(-1));

        (void) printf("Section %-4.4jd %s\n", (uintmax_t)
            elf_ndxscn(scn), name);
    }

    if ((scn = elf_getscn(e, shstrndx)) == NULL)
        errx(EXIT_FAILURE, "getscn() failed: %s.",
            elf_errmsg(-1));

    if (gelf_getshdr(scn, &shdr) != &shdr)
        errx(EXIT_FAILURE, "getshdr(shstrndx) failed: %s.",
            elf_errmsg(-1));

    (void) printf(".shstrab: size=%jd\n", (uintmax_t)
        shdr.sh_size);

    data = NULL; n = 0;
    (void) putchar('\n');

    (void) elf_end(e);
    (void) close(fd);
    exit(EXIT_SUCCESS);
}
