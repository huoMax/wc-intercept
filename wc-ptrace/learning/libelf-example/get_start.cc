/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-07-11 06:48:16
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-07-11 23:12:31
 * @FilePath: /wc-intercept/wc-ptrace/learning/libelf-example/get_start.cc
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* libelf library include */
#include <libelf.h>

int main(int argc, char **argv) {

    int fd;

    Elf *e;     // 声明一个Elf对象，用于初始化可执行文件相关信息
    char *k;

    Elf_Kind ek;    // 可执行文件的类型

    if ( argc != 2 )
        errx(EXIT_FAILURE, "usage:%s file-name", argv[0]);
    
    /* 
     * 在使用libelf之前，必须使用elf_version向库表明应用程序期望的ELF规范版本，
     * 应用程序期望的ELF版本应该和可执行文件的ELF规范版本一致，
     * 但实际上ELF版本就没变过，当前版本（EV_CURRENT）为1
     */
    if ( elf_version(EV_CURRENT) == EV_NONE )
        errx(EXIT_FAILURE, "ELF library initialization failed: %s", elf_errmsg(-1));
    
    if ( (fd=open(argv[1], O_RDONLY, 0)) < 0 )
        errx(EXIT_FAILURE, "open %s failed", argv[1]);
    
    /* 
     * elf_begin接受一个打开的文件描述符，并根据指定的参数将其转换为ELF句柄，其中第二个参数代表：
     * - ELF_C_READ：用于打开ELF对象进行读取
     * - ELF_C_WRITE：用于创建新的ELF对象
     * - ELF_C_RDWR：用于打开ELF对象进行更新
     * 文件描述符的打开模式必须要和第二参数一致，第三个参数用于处理ar archives
     */
    if ( (e=elf_begin(fd, ELF_C_READ, NULL)) == NULL )
        /*
         * libelf库内部存在错误码，可以通过elf_errno获取
         * elf_errmsg用于解释错误码，值-1表示当前错误号
         */
        errx(EXIT_FAILURE, "elf_begin() failed: %s", elf_errmsg(-1)); 
    
    /*
     * 获取ELF文件的对象类型
     */
    ek = elf_kind(e);

    switch ( ek ) {
    case ELF_K_AR:
        k = "ar(1) archive";
        break;
    case ELF_K_ELF:
        k = "elf object";
        break;
    case ELF_K_NONE:
        k = "data";
        break;
    default:
        k = "unrecognized";
        break;
    }

    printf("%s: %s\n", argv[1], k);

    /*
     * 不再使用的ELF句柄，需要使用elf_end将其释放
     */
    elf_end(e);
    close(fd);

    exit(EXIT_SUCCESS);
}
