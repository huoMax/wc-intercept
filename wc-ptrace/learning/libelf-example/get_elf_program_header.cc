/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-07-12 00:21:28
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-07-12 04:40:32
 * @FilePath: /wc-intercept/wc-ptrace/learning/libelf-example/get_elf_program_header.cc
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

void print_ptype(size_t pt){
    char * s;
    # define C(V) case PT_##V: s=#V; break
        switch ( pt ) {
            C(NULL); C(LOAD); C(DYNAMIC);
            C(INTERP); C(NOTE); C(SHLIB);
            C(PHDR); C(TLS); C(SUNWBSS); 
            C(SUNWSTACK);
            default:
                s = "unknown" ;
                break ;
        }
    printf ( " \"%s\" ", s);
    #undef C
}

int main (int argc , char **argv) {
    int i, fd ;
    Elf * e ;
    char * id , bytes[5];
    size_t n ;
    GElf_Phdr phdr ;

    if ( argc != 2)
        errx ( EXIT_FAILURE , "usage: %s file-name" , argv[0]);

    if ( elf_version ( EV_CURRENT ) == EV_NONE )
        errx( EXIT_FAILURE , "ELF library initialization failed : %s " , elf_errmsg ( -1));

    if (( fd = open(argv[1] , O_RDONLY, 0)) < 0)
        err ( EXIT_FAILURE , " open \"%s\" failed" , argv [1]);

    if (( e = elf_begin(fd, ELF_C_READ, NULL )) == NULL )
        errx ( EXIT_FAILURE , " elf_begin() failed: %s .", elf_errmsg ( -1));
    
    if (elf_kind(e) != ELF_K_ELF)
        errx ( EXIT_FAILURE , "\"%s\" is not an ELF object.", argv [1]);
    
    if ( elf_getphdrnum (e , & n ) != 0)
        errx(EXIT_FAILURE , "elf_getphdrnum() failed: %s." , elf_errmsg ( -1));
    
    for ( i = 0; i < n ; i ++) {
        if ( gelf_getphdr(e, i, &phdr ) != &phdr ) 
            errx( EXIT_FAILURE , " getphdr() failed : %s . " , elf_errmsg ( -1));
        printf ( " PHDR %d :\n", i );
        
        #define PRINT_FMT "%-20s 0x%jx"
        #define PRINT_FIELD(N) do { \
            printf ( PRINT_FMT , #N , ( uintmax_t )phdr.N ); \
        } while (0)
        # define NL() do { ( void ) printf ( "\n" ); } while (0)

        PRINT_FIELD ( p_type );

        print_ptype ( phdr.p_type ); NL ();

        PRINT_FIELD ( p_offset ); NL ();
        PRINT_FIELD ( p_vaddr ); NL ();
        PRINT_FIELD ( p_paddr ); NL ();
        PRINT_FIELD ( p_filesz ); NL ();
        PRINT_FIELD ( p_memsz ); NL ();
        PRINT_FIELD ( p_flags );

        printf ( " [ " );
        if (phdr.p_flags &PF_X )
        printf ( "execute" );
        if ( phdr.p_flags &PF_R )
        printf ( " read " );
        if( phdr.p_flags &PF_W )
        printf ( " write " );
        printf ( " ] " ); NL ();
        PRINT_FIELD ( p_align ); NL ();
    }

    elf_end ( e );
    close ( fd );
    exit ( EXIT_SUCCESS );
}
