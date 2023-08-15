# ELF文件格式与Libelf解析

## 一、ELF文件格式

对于ELF文件的介绍，我推荐去阅读《程序员的自我修养——链接、装载与库》，虽然里面只介绍了理论部分，并且里面的ELF格式放在现在已经不太适用，但依旧是一本优秀的启蒙书。

ELF（Executable and Linkable Format，可执行可链接格式）是一种用于描述二进制文件格式的标准，它在 Unix 和 Unix-like 操作系统中得到广泛应用。ELF 文件格式定义了可执行文件、共享库和目标文件的内部结构，包括代码、数据、符号表、重定位表、动态链接信息等。

ELF 文件被广泛应用于 Unix 和 Unix-like 操作系统中的程序开发和部署中，如 Linux、FreeBSD、macOS 等系统。ELF 文件格式具有可扩展性和可移植性，支持多种体系结构和操作系统，并且具有较好的兼容性和可靠性。我们熟悉的C/C++、go等编译后的可执行文件在Linux上就表现为ELF文件格式。

接下来我会介绍ELF文件的主要结构，主要是基于glibc中关于ELF格式的数据结构定义，定义在`/usr/include/elf.h`.

下面是一个示例程序：

```c
/*
 * tracee.c
 */
#include <stdio.h>
#include <stdlib.h>

void test(){
    int a=0, b=1;
    printf("test\n");
}

int main () {
    printf("hello world, I will rand a: %d\n", rand());
    printf("byby, I will rand a: %d\n", rand());
    test();
    return 0;
}
```

编译（编译器默认启用了延迟绑定和动态链接）：

```shell
gcc tracee.c -o tracee
```

[ELF文件的格式](https://www.uclibc.org/docs/elf-64-gen.pdf)如下所示：

![image-20230710164516074](assets/ELF文件格式.png)

我们将会结合glibc的定义和示例程序来介绍ELF文件的主要部分。

如果想要了解更多、更详细的信息，推荐阅读ELF官方标准[4],[5]：

* 《ELF-64 Object File Format》
* 《Tool Interface Standard (TIS) Executable and Linking Format (ELF) Specification Version 1.2》

### 1.1 ELF Header

ELF 头（ELF Header）是 ELF 文件的第一个段，它包含了一些重要的信息，如文件类型、机器架构、入口地址、节头表偏移量等等。

glibc中的ELF Header定义如下：

```c++
typedef struct
{
  unsigned char	e_ident[EI_NIDENT];		/* 文件魔数，用于识别文件类型 */
  Elf64_Half	e_type;				   /* 标识ELF文件类型 */
  Elf64_Half	e_machine;			   /* 标识ELF文件所运行的系统架构 */
  Elf64_Word	e_version;		       /* ELF文件的版本号 */
  Elf64_Addr	e_entry;		       /* ELF文件入口地址 */
  Elf64_Off	e_phoff;				  /* Program Header Table的偏移量 */
  Elf64_Off	e_shoff;				  /* Section Header Table的偏移量 */
  Elf64_Word	e_flags;			  /* 描述ELF文件的一些特性和属性 */
  Elf64_Half	e_ehsize;			  /* ELF Header的大小 */
  Elf64_Half	e_phentsize;		  /* Program Header的大小 */
  Elf64_Half	e_phnum;			  /* Program Header Table的表项数量 */
  Elf64_Half	e_shentsize;		  /* Section header table的大小 */
  Elf64_Half	e_shnum;			  /* Section header table的表项数量 */
  Elf64_Half	e_shstrndx;		      /* 标识.strtab(字符串表)在Section Header Table中的索引 */
} Elf64_Ehdr;
```

通过readelf可以查看一个可执行文件的头部：

```shell
huomax@huomax-ubuntu:~/wgk/wc-intercept/wc-ptrace/learning/delay_bind$ readelf -h tracee
ELF 头：
  Magic：   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
  类别:                              ELF64
  数据:                              2 补码，小端序 (little endian)
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI 版本:                          0
  类型:                              DYN (Position-Independent Executable file)
  系统架构:                          Advanced Micro Devices X86-64
  版本:                              0x1
  入口点地址：               0x10a0
  程序头起点：          64 (bytes into file)
  Start of section headers:          14136 (bytes into file)
  标志：             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         13
  Size of section headers:           64 (bytes)
  Number of section headers:         31
  Section header string table index: 30
```

### 1.2 Program Header Table

程序头表（Program Header Table），对于可加载文件是必需的，对于可重定位文件是可选的。该表描述了加载程序或动态链接库以准备执行所需的可加载段和其他数据结构。其主要作用是描述 ELF 文件在内存中的运行时映像。程序头表记录了 ELF 文件中每个可加载的段（Loadable Segment）在内存中的位置、大小和属性等信息，以便操作系统在加载和执行 ELF 文件时正确地分配内存空间，将每个段映射到相应的内存区域中。

注意：在ELF文件中，段（segment）和节（section）是两个不同的概念。ELF文件在链接时为了满足程序在内存中的需求（可参考程序员的自我修养4.1节），将多个相同类型的节合并为一个段。

在glibc中Program Header Table中每个表项的定义如下：

```c++
typedef struct
{
  Elf64_Word	p_type;			/* 段类型 */
  Elf64_Word	p_flags;		/* 段的标志 */
  Elf64_Off	p_offset;		    /* 该段在ELF文件中的偏移量 */
  Elf64_Addr	p_vaddr;		/* 该段在内存中的虚拟地址 */
  Elf64_Addr	p_paddr;		/* 该段在物理内存中的地址，大部分情况下该字段未使用 */
  Elf64_Xword	p_filesz;		/* 该段在ELF中的大小 */
  Elf64_Xword	p_memsz;		/* 该段在内存中的大小 */
  Elf64_Xword	p_align;		/* 该段在文件和内存中的对齐方式 */
} Elf64_Phdr;
```

注意：p_vaddr字段虽然说是该段在内存中的虚拟地址，但其实该段被加载到内存的首地址应该为：进程加载基地址+p_vaddr。

通过readelf可以看到可执行文件的程序头表

```shell
huomax@huomax-ubuntu:~/wgk/wc-intercept/wc-ptrace/learning/delay_bind$ readelf -l tracee

Elf 文件类型为 DYN (Position-Independent Executable file)
Entry point 0x10a0
There are 13 program headers, starting at offset 64

程序头：
  Type           Offset             VirtAddr           PhysAddr
                 FileSiz            MemSiz              Flags  Align
  PHDR           0x0000000000000040 0x0000000000000040 0x0000000000000040
                 0x00000000000002d8 0x00000000000002d8  R      0x8
  INTERP         0x0000000000000318 0x0000000000000318 0x0000000000000318
                 0x000000000000001c 0x000000000000001c  R      0x1
      [Requesting program interpreter: /lib64/ld-linux-x86-64.so.2]
  LOAD           0x0000000000000000 0x0000000000000000 0x0000000000000000
                 0x0000000000000698 0x0000000000000698  R      0x1000
  LOAD           0x0000000000001000 0x0000000000001000 0x0000000000001000
                 0x0000000000000211 0x0000000000000211  R E    0x1000
  LOAD           0x0000000000002000 0x0000000000002000 0x0000000000002000
                 0x0000000000000154 0x0000000000000154  R      0x1000
  LOAD           0x0000000000002da8 0x0000000000003da8 0x0000000000003da8
                 0x000000000000026c 0x0000000000000270  RW     0x1000
  DYNAMIC        0x0000000000002db8 0x0000000000003db8 0x0000000000003db8
                 0x00000000000001f0 0x00000000000001f0  RW     0x8
  NOTE           0x0000000000000338 0x0000000000000338 0x0000000000000338
                 0x0000000000000030 0x0000000000000030  R      0x8
  NOTE           0x0000000000000368 0x0000000000000368 0x0000000000000368
                 0x0000000000000044 0x0000000000000044  R      0x4
  GNU_PROPERTY   0x0000000000000338 0x0000000000000338 0x0000000000000338
                 0x0000000000000030 0x0000000000000030  R      0x8
  GNU_EH_FRAME   0x000000000000204c 0x000000000000204c 0x000000000000204c
                 0x000000000000003c 0x000000000000003c  R      0x4
  GNU_STACK      0x0000000000000000 0x0000000000000000 0x0000000000000000
                 0x0000000000000000 0x0000000000000000  RW     0x10
  GNU_RELRO      0x0000000000002da8 0x0000000000003da8 0x0000000000003da8
                 0x0000000000000258 0x0000000000000258  R      0x1

 Section to Segment mapping:
  段节...
   00     
   01     .interp 
   02     .interp .note.gnu.property .note.gnu.build-id .note.ABI-tag .gnu.hash .dynsym .dynstr .gnu.version .gnu.version_r .rela.dyn .rela.plt 
   03     .init .plt .plt.got .plt.sec .text .fini 
   04     .rodata .eh_frame_hdr .eh_frame 
   05     .init_array .fini_array .dynamic .got .data .bss 
   06     .dynamic 
   07     .note.gnu.property 
   08     .note.gnu.build-id .note.ABI-tag 
   09     .note.gnu.property 
   10     .eh_frame_hdr 
   11     
   12     .init_array .fini_array .dynamic .got 
```

### 1.3 Section Header Table

Section table（节表）是可执行文件中的一种重要数据结构，用于描述可执行文件中各个节（Section）的信息。每个节代表可执行文件中的一个区域，包含了不同类型的信息，如代码、数据、符号表、字符串常量等。对于可重定位文件是必需的，对于可加载文件是可选的。

Section table的作用主要有以下几个方面：

1. 描述可执行文件中各个节的信息：Section table记录了可执行文件中每个节的起始地址、大小、类型、属性等信息，可以让操作系统和加载器根据这些信息来正确加载和映射可执行文件到内存中。
2. 优化可执行文件的大小和性能：Section table中的节信息可以被编译器和链接器用来进行优化，例如去掉未使用的代码和数据，合并相邻的节，压缩节的内容等，以减小可执行文件的大小和提高运行时的性能。
3. 支持动态链接：Section table中的某些节信息可以被动态链接器用来加载和解析动态链接库，例如动态符号表、重定位表、PLT和GOT等，以支持动态链接和运行时的符号解析。

Section Header Table的每个表项在glibc中的定义如下：

```c++
typedef struct
{
  Elf64_Word	sh_name;		/* 节的名称，但实际上是在一段地址，是在.strtab中的索引 */
  Elf64_Word	sh_type;		/* 节的类型 */
  Elf64_Xword	sh_flags;		/* 节的标志 */
  Elf64_Addr	sh_addr;		/* 节的虚拟地址 */
  Elf64_Off	sh_offset;			/* 节在ELF中的偏移量 */
  Elf64_Xword	sh_size;		/* 节的大小 */
  Elf64_Word	sh_link;		/* 该字段是一个节索引，指定了该节相关的附加信息所在的节的索引。 */
  Elf64_Word	sh_info;		/* 附加信息 */
  Elf64_Xword	sh_addralign;	/* 节的对齐方式 */
  Elf64_Xword	sh_entsize;		/* 某些节类型的每个条目（entry）的大小 */
} Elf64_Shdr;
```

通过readelf查看Section Header Table：

```c++
huomax@huomax-ubuntu:~/wgk/wc-intercept/wc-ptrace/learning/delay_bind$ readelf -S tracee
There are 31 section headers, starting at offset 0x3738:

节头：
  [号] 名称              类型             地址              偏移量
       大小              全体大小          旗标   链接   信息   对齐
  [ 0]                   NULL             0000000000000000  00000000
       0000000000000000  0000000000000000           0     0     0
  [ 1] .interp           PROGBITS         0000000000000318  00000318
       000000000000001c  0000000000000000   A       0     0     1
  [ 2] .note.gnu.pr[...] NOTE             0000000000000338  00000338
       0000000000000030  0000000000000000   A       0     0     8
  [ 3] .note.gnu.bu[...] NOTE             0000000000000368  00000368
       0000000000000024  0000000000000000   A       0     0     4
  [ 4] .note.ABI-tag     NOTE             000000000000038c  0000038c
       0000000000000020  0000000000000000   A       0     0     4
  [ 5] .gnu.hash         GNU_HASH         00000000000003b0  000003b0
       0000000000000024  0000000000000000   A       6     0     8
  [ 6] .dynsym           DYNSYM           00000000000003d8  000003d8
       00000000000000d8  0000000000000018   A       7     1     8
  [ 7] .dynstr           STRTAB           00000000000004b0  000004b0
       0000000000000099  0000000000000000   A       0     0     1
  [ 8] .gnu.version      VERSYM           000000000000054a  0000054a
       0000000000000012  0000000000000002   A       6     0     2
  [ 9] .gnu.version_r    VERNEED          0000000000000560  00000560
       0000000000000030  0000000000000000   A       7     1     8
  [10] .rela.dyn         RELA             0000000000000590  00000590
       00000000000000c0  0000000000000018   A       6     0     8
  [11] .rela.plt         RELA             0000000000000650  00000650
       0000000000000048  0000000000000018  AI       6    24     8
  [12] .init             PROGBITS         0000000000001000  00001000
       000000000000001b  0000000000000000  AX       0     0     4
  [13] .plt              PROGBITS         0000000000001020  00001020
       0000000000000040  0000000000000010  AX       0     0     16
  [14] .plt.got          PROGBITS         0000000000001060  00001060
       0000000000000010  0000000000000010  AX       0     0     16
  [15] .plt.sec          PROGBITS         0000000000001070  00001070
       0000000000000030  0000000000000010  AX       0     0     16
  [16] .text             PROGBITS         00000000000010a0  000010a0
       0000000000000164  0000000000000000  AX       0     0     16
  [17] .fini             PROGBITS         0000000000001204  00001204
       000000000000000d  0000000000000000  AX       0     0     4
  [18] .rodata           PROGBITS         0000000000002000  00002000
       0000000000000049  0000000000000000   A       0     0     8
  [19] .eh_frame_hdr     PROGBITS         000000000000204c  0000204c
       000000000000003c  0000000000000000   A       0     0     4
  [20] .eh_frame         PROGBITS         0000000000002088  00002088
       00000000000000cc  0000000000000000   A       0     0     8
  [21] .init_array       INIT_ARRAY       0000000000003da8  00002da8
       0000000000000008  0000000000000008  WA       0     0     8
  [22] .fini_array       FINI_ARRAY       0000000000003db0  00002db0
       0000000000000008  0000000000000008  WA       0     0     8
  [23] .dynamic          DYNAMIC          0000000000003db8  00002db8
       00000000000001f0  0000000000000010  WA       7     0     8
  [24] .got              PROGBITS         0000000000003fa8  00002fa8
       0000000000000058  0000000000000008  WA       0     0     8
  [25] .data             PROGBITS         0000000000004000  00003000
       0000000000000014  0000000000000000  WA       0     0     8
  [26] .bss              NOBITS           0000000000004014  00003014
       0000000000000004  0000000000000000  WA       0     0     1
  [27] .comment          PROGBITS         0000000000000000  00003014
       000000000000002d  0000000000000001  MS       0     0     1
  [28] .symtab           SYMTAB           0000000000000000  00003048
       00000000000003c0  0000000000000018          29    18     8
  [29] .strtab           STRTAB           0000000000000000  00003408
       0000000000000210  0000000000000000           0     0     1
  [30] .shstrtab         STRTAB           0000000000000000  00003618
       000000000000011a  0000000000000000           0     0     1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  D (mbind), l (large), p (processor specific)
```

### 1.4 Sections

每个ELF文件包含了许多的节，如我们熟知的代码段（.text），只读段（.rodata），数据段（.data）。我们会大致介绍对于ELF文件来说比较重要的一些节：

`.text`

`.text`节是保存了程序代码指令的**代码节**。**一段可执行程序，如果存在Phdr，则`.text`节就会存在于`text`段中**。由于`.text`节保存了程序代码，所以节类型为`SHT_PROGBITS`。

`.rodata`

`rodata`节保存了只读的数据，如一行C语言代码中的字符串。由于`.rodata`节是只读的，所以只能存在于一个可执行文件的**只读段**中。因此，只能在`text`段（不是`data`段）中找到`.rodata`节。由于`.rodata`节是只读的，所以节类型为`SHT_PROGBITS`。

`.plt`

`.plt`节也称为**过程链接表（Procedure Linkage Table）**，**其包含了动态链接器调用从共享库导入的函数所必需的相关代码**。由于`.plt`节保存了代码，所以节类型为`SHT_PROGBITS`。

`.data`

`.data`节存在于`data`段中，**其保存了初始化的全局变量等数据**。由于`.data`节保存了程序的变量数据，所以节类型为`SHT_PROGBITS`。

`.bss`

`.bss`节存在于`data`段中，占用空间不超过4字节，仅表示这个节本省的空间。**`.bss`节保存了未进行初始化的全局数据**。程序加载时数据被初始化为0，在程序执行期间可以进行赋值。由于`.bss`节未保存实际的数据，所以节类型为`SHT_NOBITS`。

`.got.plt`

`.got`节保存了**全局偏移表**。**`.got`节和`.plt`节一起提供了对导入的共享库函数的访问入口，由动态链接器在运行时进行修改**。由于`.got.plt`节与程序执行有关，所以节类型为`SHT_PROGBITS`。

`.dynsym`

`.dynsym`节保存在`text`段中。**其保存了从共享库导入的动态符号表**。节类型为`SHT_DYNSYM`。

`.dynstr`

`.dynstr`保存了动态链接字符串表，表中存放了一系列字符串，这些字符串代表了符号名称，以空字符作为终止符。

`.rela.*`

重定位表保存了重定位相关的信息，**这些信息描述了如何在链接或运行时，对ELF目标文件的某部分或者进程镜像进行补充或修改**。由于重定位表保存了重定位相关的数据，所以节类型为`SHT_REL`。

`.hash`

`.hash`节也称为`.gnu.hash`，其保存了一个用于查找符号的散列表。

`.symtab`

`.symtab`节保存了符号信息。节类型为`SHT_SYMTAB`。

`.strtab`

字符串表节包含用于节名和符号名的字符串。字符串表只是包含以空结束的字符串的字节数组。

正如我们在Section Header Table所说，每个节的节名字段sh_name只是一个地址，表示节的名称在.strtab中的偏移量，字符串表中的第一个字节被定义为null，因此索引0总是指向null或不存在的名称。

`.ctors、.dtors`

`.ctors`（**构造器**）节和`.dtors`（**析构器**）节分别保存了指向构造函数和析构函数的函数指针，**构造函数是在main函数执行之前需要执行的代码；析构函数是在main函数之后需要执行的代码**。

#### 1.4.1 .symtab

符号表存储了ELF文件中的所有符号（函数、变量），在glibc中的定义如下：

```c++
typedef struct
{
  Elf64_Word	st_name;		/* 符号名称，在.strtab中的索引 */
  unsigned char	st_info;		/* 符号类型和信息 */
  unsigned char st_other;		/* 保留字段，尚未使用 */
  Elf64_Section	st_shndx;		/* 符号所在节的索引 */
  Elf64_Addr	st_value;		/* 符号的值或地址 */
  Elf64_Xword	st_size;		/* 符号大小 */
} Elf64_Sym;
```

通过readelf查看可执行文件的符号表信息：

```c++
huomax@huomax-ubuntu:~/wgk/wc-intercept/wc-ptrace/learning/delay_bind$ readelf -s tracee

Symbol table '.dynsym' contains 9 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND _[...]@GLIBC_2.34 (2)
     2: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_deregisterT[...]
     3: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND puts@GLIBC_2.2.5 (3)
     4: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND [...]@GLIBC_2.2.5 (3)
     5: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND __gmon_start__
     6: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_registerTMC[...]
     7: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND rand@GLIBC_2.2.5 (3)
     8: 0000000000000000     0 FUNC    WEAK   DEFAULT  UND [...]@GLIBC_2.2.5 (3)

Symbol table '.symtab' contains 40 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS Scrt1.o
     2: 000000000000038c    32 OBJECT  LOCAL  DEFAULT    4 __abi_tag
     3: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS crtstuff.c
     4: 00000000000010d0     0 FUNC    LOCAL  DEFAULT   16 deregister_tm_clones
     5: 0000000000001100     0 FUNC    LOCAL  DEFAULT   16 register_tm_clones
     6: 0000000000001140     0 FUNC    LOCAL  DEFAULT   16 __do_global_dtors_aux
     7: 0000000000004014     1 OBJECT  LOCAL  DEFAULT   26 completed.0
     8: 0000000000003db0     0 OBJECT  LOCAL  DEFAULT   22 __do_global_dtor[...]
     9: 0000000000001180     0 FUNC    LOCAL  DEFAULT   16 frame_dummy
    10: 0000000000003da8     0 OBJECT  LOCAL  DEFAULT   21 __frame_dummy_in[...]
    11: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS tracee.c
    12: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS crtstuff.c
    13: 0000000000002150     0 OBJECT  LOCAL  DEFAULT   20 __FRAME_END__
    14: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS 
    15: 0000000000003db8     0 OBJECT  LOCAL  DEFAULT   23 _DYNAMIC
    16: 000000000000204c     0 NOTYPE  LOCAL  DEFAULT   19 __GNU_EH_FRAME_HDR
    17: 0000000000003fa8     0 OBJECT  LOCAL  DEFAULT   24 _GLOBAL_OFFSET_TABLE_
    18: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND __libc_start_mai[...]
    19: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_deregisterT[...]
    20: 0000000000004000     0 NOTYPE  WEAK   DEFAULT   25 data_start
    21: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND puts@GLIBC_2.2.5
    22: 0000000000004014     0 NOTYPE  GLOBAL DEFAULT   25 _edata
    23: 0000000000001204     0 FUNC    GLOBAL HIDDEN    17 _fini
    24: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND printf@GLIBC_2.2.5
    25: 0000000000004000     0 NOTYPE  GLOBAL DEFAULT   25 __data_start
    26: 0000000000004010     4 OBJECT  GLOBAL DEFAULT   25 var_global
    27: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND __gmon_start__
    28: 0000000000004008     0 OBJECT  GLOBAL HIDDEN    25 __dso_handle
    29: 0000000000002000     4 OBJECT  GLOBAL DEFAULT   18 _IO_stdin_used
    30: 0000000000004018     0 NOTYPE  GLOBAL DEFAULT   26 _end
    31: 00000000000010a0    38 FUNC    GLOBAL DEFAULT   16 _start
    32: 0000000000004014     0 NOTYPE  GLOBAL DEFAULT   26 __bss_start
    33: 00000000000011b5    79 FUNC    GLOBAL DEFAULT   16 main
    34: 0000000000004018     0 OBJECT  GLOBAL HIDDEN    25 __TMC_END__
    35: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_registerTMC[...]
    36: 0000000000000000     0 FUNC    WEAK   DEFAULT  UND __cxa_finalize@G[...]
    37: 0000000000001000     0 FUNC    GLOBAL HIDDEN    12 _init
    38: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND rand@GLIBC_2.2.5
    39: 0000000000001189    44 FUNC    GLOBAL DEFAULT   16 test
```

第一个符号表项是保留的，必须全为零。符号常量STN_UNDEF用来引用这个表项。

`sy_info`

st_info字段包含符号类型及其绑定属性(即其作用域)。绑定属性包含在字节的高4位中，符号类型包含在低4位中。

ST_BIND是st_info中的绑定信息（高四位），表示符号的绑定类型，即符号的可见性和链接范围。ST_BIND 字段的取值和对应的解释如下：

| Name           | Value | Meaning                                                      |
| -------------- | ----- | ------------------------------------------------------------ |
| STB_LOCAL      | 0     | 局部符号，只在当前模块内可见                                 |
| STB_GLOBAL     | 1     | 全局符号，可被其他模块引用，同时也可被本模块引用             |
| STB_WEAK       | 2     | 弱符号，类似全局符号，但是如果全局符号不存在，则使用弱符号，且弱符号在链接时可以被覆盖 |
| STB_NUM        | 3     | 保留的符号绑定类型，表示符号绑定类型的数量                   |
| STB_LOOS       | 10    | 保留的符号绑定类型，特定于操作系统                           |
| STB_GNU_UNIQUE | 10    | 全局唯一符号，整个程序中只有一个实例                         |
| STB_HIOS       | 12    | 保留的符号绑定类型，特定于操作系统或架构                     |
| STB_LOPROC     | 13    | 保留的符号绑定类型，特定于操作系统或架构                     |
| STB_HIPROC     | 15    | 保留的符号绑定类型，特定于操作系统或架构                     |

ST_TYPE是st_info中的类型信息（低四位），取值如下：

| Name          | Value | Meaning                                                      |
| ------------- | ----- | ------------------------------------------------------------ |
| STT_NOTYPE    | 0     | 未知类型，表示该符号没有特定的类型信息，例如使用绝对地址的符号 |
| STT_OBJECT    | 1     | 对象类型，表示该符号是一个变量或者数据对象                   |
| STT_FUNC      | 2     | 函数类型，表示该符号是一个函数                               |
| STT_SECTION   | 3     | 节类型，表示该符号是一个节的符号表条目                       |
| STT_FILE      | 4     | 文件类型，表示该符号是一个文件名。                           |
| STT_COMMON    | 5     | 普通的未初始化数据符号，通常用于的未初始化全局变量和静态变量 |
| STT_TLS       | 6     | 该符号在程序中占用的内存空间是线程本地的，即线程局部变量     |
| STT_NUM       | 7     | 定义的符号类型数量                                           |
| STT_LOOS      | 10    | 保留的符号类型，特定于操作系统或架构                         |
| STT_GNU_IFUNC | 10    | GNU 工具链扩展定义的一种特殊的符号类型，用于定义一个间接函数 |
| STT_HIOS      | 12    | 保留的符号类型，特定于操作系统或架构                         |
| STT_LOPROC    | 13    | 保留的符号类型，特定于操作系统或架构                         |
| STT_HIPROC    | 15    | 保留的符号类型，特定于操作系统或架构                         |

`st_shndx`

st_shndx 字段表示该符号所属的section的索引，该字段取值如下：

| Name          | Value  | Meaning                                                      |
| ------------- | ------ | ------------------------------------------------------------ |
| SHN_UNDEF     | 0      | 未定义节，说明该符号没有在当前目标文件中定义，需要在其他目标文件中查找该符号的定义。 |
| SHN_LORESERVE | 0xff00 | Start of reserved indices                                    |
| SHN_LOPROC    | 0xff00 | Start of processor-specific                                  |
| SHN_BEFORE    | 0xff00 | Order section before all others (Solaris)                    |
| SHN_AFTER     | 0xff01 | Order section after all others (Solaris)                     |
| SHN_HIPROC    | 0xff1f | End of processor-specific                                    |
| SHN_LOOS      | 0xff20 | Start of OS-specific                                         |
| SHN_HIOS      | 0xff3f | End of OS-specific                                           |
| SHN_ABS       | 0xfff1 | 该符号所属节的地址是绝对地址，通常用于需要在多个目标文件中共享的符号，例如全局常量。 |
| SHN_COMMON    | 0xfff2 | 表示该符号是一个未初始化的数据对象（Common Symbol），所属节是未初始化数据节。 |
| SHN_XINDEX    | 0xffff | Index is in extra table.                                     |
| SHN_HIRESERVE | 0xffff | End of reserved indices                                      |

在链接时，如果某个符号的所属节为 SHN_UNDEF，则说明该符号没有在当前目标文件中定义，需要在其他目标文件中查找该符号的定义。如果在链接过程中仍然无法找到该符号的定义，则链接过程会失败。SHN_UNDEF 通常用于外部定义的符号和未定义的符号。外部定义的符号是在当前目标文件中引用，但是定义在其他目标文件中的符号，例如库函数；未定义的符号是在当前目标文件中引用，但是没有在任何目标文件中定义的符号，例如全局变量和函数。

SHN_ABS 通常用于需要在多个目标文件中共享的符号，例如全局常量。由于全局常量的值在编译时已经确定，并且在不同的目标文件中都是相同的，因此可以将全局常量的地址设置为一个绝对地址，从而在不同的目标文件中共享该符号。也就是说如果一个符号是SHN_ABS的，那么它在进程中的加载地址为st_value，而普通的符号在进程中的加载地址为：st_value+进程的加载基地址。

Common Symbol 是一种特殊的符号类型，它表示一块未初始化的共享数据，多个目标文件中同名的 Common Symbol 会在链接时被合并为一个符号，并且共享同一个内存空间。在 ELF 文件中，所有的 Common Symbol 都被放在一个特殊的未初始化数据节（BSS Section）中，该节的节头信息通常被命名为 .bss。

#### 1.4.2 .rela.*

ELF格式定义了两种标准的重定位格式，“Rel”和“Rela”。第一种形式较短，从被重新定位的单词的原始值中获得重新定位的加数部分。第二种形式为全宽加数提供显式字段。通过readelf看到重定位表：

```c++
huomax@huomax-ubuntu:~/wgk/wc-intercept/wc-ptrace/learning/delay_bind$ readelf -r tracee

重定位节 '.rela.dyn' at offset 0x590 contains 8 entries:
  偏移量          信息           类型           符号值        符号名称 + 加数
000000003da8  000000000008 R_X86_64_RELATIVE                    1180
000000003db0  000000000008 R_X86_64_RELATIVE                    1140
000000004008  000000000008 R_X86_64_RELATIVE                    4008
000000003fd8  000100000006 R_X86_64_GLOB_DAT 0000000000000000 __libc_start_main@GLIBC_2.34 + 0
000000003fe0  000200000006 R_X86_64_GLOB_DAT 0000000000000000 _ITM_deregisterTM[...] + 0
000000003fe8  000500000006 R_X86_64_GLOB_DAT 0000000000000000 __gmon_start__ + 0
000000003ff0  000600000006 R_X86_64_GLOB_DAT 0000000000000000 _ITM_registerTMCl[...] + 0
000000003ff8  000800000006 R_X86_64_GLOB_DAT 0000000000000000 __cxa_finalize@GLIBC_2.2.5 + 0

重定位节 '.rela.plt' at offset 0x650 contains 3 entries:
  偏移量          信息           类型           符号值        符号名称 + 加数
000000003fc0  000300000007 R_X86_64_JUMP_SLO 0000000000000000 puts@GLIBC_2.2.5 + 0
000000003fc8  000400000007 R_X86_64_JUMP_SLO 0000000000000000 printf@GLIBC_2.2.5 + 0
000000003fd0  000700000007 R_X86_64_JUMP_SLO 0000000000000000 rand@GLIBC_2.2.5 + 0
```

.rel.*节已经被废弃，不怎么常见了，更普遍使用的是.rela.\*节，如上面的.rela.dyn和.rela.plt（用于重定位函数）。

表项在glibc中的定义为：

```c++
typedef struct
{
  Elf64_Addr	r_offset;		/* 重定位地址 */
  Elf64_Xword	r_info;			/* 包含符号表索引和重定位类型 */
  Elf64_Sxword	r_addend;		/* 指定一个常量加数，用于计算要存储在重新定位字段中的值 */
} Elf64_Rela;
```

`r_offset`字段标识重定位地址，对于一个可重定位的文件，这是一个偏移量，以字节为单位，从段的开始到被重定位的存储单元的开始。对于可执行对象或共享对象，这是要重新定位的存储单元的虚拟地址。

对于使用了延迟绑定技术的可执行对象，里面的值是PLT程序的地址，只有在函数第一次被调用是，才会通过PLT程序解析函数在动态库的地址并绑定回填到该函数符号上。

其中`r_inifo`字段标识符号表索引标识在重定位中应该使用其值的符号。

重定位类型是特定于处理器的。符号表索引是通过对该字段应用ELF64_R_SYM宏获得的，重定位类型是通过对该字段应用ELF64_R_TYPE宏获得的。ELF64_R_INFO宏结合了符号表索引和重定位类型来生成该字段的值。这些宏的定义如下：

```c++
#define ELF64_R_SYM(i)((i) >> 32)
#define ELF64_R_TYPE(i)((i) & 0xf f f f f f f f L)
#define ELF64_R_INFO(s, t)(((s) << 32) + ((t) & 0xf f f f f f f f L))
```

#### 1.4.3 .got

.got节（Global Offset Table）是ELF格式中的一个节，它是用于实现全局偏移表的节。全局偏移表是一个数据结构，用于在程序运行时实现全局变量和函数的位置无关性。在编译和链接时，全局变量和函数的地址是未知的，只有在程序运行时才能确定。因此，需要使用全局偏移表来实现这种位置无关性。

.got节存储了全局偏移表的内容，包括全局变量和函数的地址。在程序运行时，动态链接器会将全局偏移表中的地址修改为实际的地址，以实现位置无关性。

`.got`和`.rela.plt`被用来实现延迟绑定技术，如果我们对一个可执行文件进行解析，会发现`.rela.plt`中的需要重定位的动态库函数地址，其实是在`.got`节中。如printf函数，我们在上一节中看到它在`.rela.plt`中的偏移量为：0x000000003fc8，此时我们通过objdump查看`.got`节：

```c++
huomax@huomax-ubuntu:~/wgk/wc-intercept/wc-ptrace/learning/delay_bind$ objdump -s -j .got tracee

tracee：     文件格式 elf64-x86-64

Contents of section .got:
 3fa8 b83d0000 00000000 00000000 00000000  .=..............
 3fb8 00000000 00000000 30100000 00000000  ........0.......
 3fc8 40100000 00000000 50100000 00000000  @.......P.......
 3fd8 00000000 00000000 00000000 00000000  ................
 3fe8 00000000 00000000 00000000 00000000  ................
 3ff8 00000000 00000000                    ........ 
```

`.got`节前几项为系统相关的表项，我们可以看到printf的重定位偏移量指向了`.got`中的内容，为：40100000 00000000，考虑到Linux使用了小端存储，所以printf的重定位地址应该是：0x00000000 00001040。当然，我们知道它其实是指向了PLT程序。

#### 1.4.4 .dynamic

.dynamic 段是 ELF 文件中的一个动态段，它包含了程序运行时需要的动态链接信息。当程序加载到内存中时，动态链接器会读取 .dynamic 段的内容，根据其中的信息来加载动态链接库并进行符号解析。.dynamic 段的作用是将静态编译的程序转变为可运行的动态程序，从而使程序能够充分利用动态链接库的优势。

.dynamic 段包含了许多动态链接信息，包括：

- 动态链接库名称（DT_NEEDED）
- 全局符号表（DT_SYMTAB）
- 字符串表（DT_STRTAB）
- 重定位表（DT_JMPREL 和 DT_RELA）
- 符号哈希表（DT_HASH）
- 动态链接器入口地址（DT_INIT 和 DT_FINI）
- 动态链接器版本信息（DT_VERDEF 和 DT_VERNEED）
- 标志位（DT_FLAGS）

.dynamic的表项结构为：

```c++
typedef struct
{
  Elf64_Sxword	d_tag;			/* Dynamic entry type */
  union
    {
      Elf64_Xword d_val;		/* Integer value */
      Elf64_Addr d_ptr;			/* Address value */
    } d_un;
} Elf64_Dyn;
```

其中`d_tag`表示表项类型，有以下取值：

```c++
/* Legal values for d_tag (dynamic entry type).  */

#define DT_NULL		0		/* Marks end of dynamic section */
#define DT_NEEDED	1		/* Name of needed library */
#define DT_PLTRELSZ	2		/* Size in bytes of PLT relocs */
#define DT_PLTGOT	3		/* Processor defined value */
#define DT_HASH		4		/* Address of symbol hash table */
#define DT_STRTAB	5		/* Address of string table */
#define DT_SYMTAB	6		/* Address of symbol table */
#define DT_RELA		7		/* Address of Rela relocs */
#define DT_RELASZ	8		/* Total size of Rela relocs */
#define DT_RELAENT	9		/* Size of one Rela reloc */
#define DT_STRSZ	10		/* Size of string table */
#define DT_SYMENT	11		/* Size of one symbol table entry */
#define DT_INIT		12		/* Address of init function */
#define DT_FINI		13		/* Address of termination function */
#define DT_SONAME	14		/* Name of shared object */
#define DT_RPATH	15		/* Library search path (deprecated) */
#define DT_SYMBOLIC	16		/* Start symbol search here */
#define DT_REL		17		/* Address of Rel relocs */
#define DT_RELSZ	18		/* Total size of Rel relocs */
#define DT_RELENT	19		/* Size of one Rel reloc */
#define DT_PLTREL	20		/* Type of reloc in PLT */
#define DT_DEBUG	21		/* For debugging; unspecified */
#define DT_TEXTREL	22		/* Reloc might modify .text */
#define DT_JMPREL	23		/* Address of PLT relocs */
#define	DT_BIND_NOW	24		/* Process relocations of object */
#define	DT_INIT_ARRAY	25		/* Array with addresses of init fct */
#define	DT_FINI_ARRAY	26		/* Array with addresses of fini fct */
#define	DT_INIT_ARRAYSZ	27		/* Size in bytes of DT_INIT_ARRAY */
#define	DT_FINI_ARRAYSZ	28		/* Size in bytes of DT_FINI_ARRAY */
#define DT_RUNPATH	29		/* Library search path */
#define DT_FLAGS	30		/* Flags for the object being loaded */
#define DT_ENCODING	32		/* Start of encoded range */
#define DT_PREINIT_ARRAY 32		/* Array with addresses of preinit fct*/
#define DT_PREINIT_ARRAYSZ 33		/* size in bytes of DT_PREINIT_ARRAY */
#define DT_SYMTAB_SHNDX	34		/* Address of SYMTAB_SHNDX section */
#define	DT_NUM		35		/* Number used */
#define DT_LOOS		0x6000000d	/* Start of OS-specific */
#define DT_HIOS		0x6ffff000	/* End of OS-specific */
#define DT_LOPROC	0x70000000	/* Start of processor-specific */
#define DT_HIPROC	0x7fffffff	/* End of processor-specific */
#define	DT_PROCNUM	DT_MIPS_NUM	/* Most used by any processor */
```

内容较多我只介绍一些使用过的类型：

`DT_BIND_NOW`

```c++
#define	DT_BIND_NOW	24		/* Process relocations of object */
```

DF_BIND_NOW 是 ELF 文件中的一个标志位，它指示程序是否在链接时进行符号绑定。如果 DF_BIND_NOW 的值为 1，表示程序在链接时立即进行符号绑定；否则，程序在需要时才进行符号绑定。

当d_tag是`DT_BIND_NOW`时，d_val字段是 DT_BIND_NOW 条目在动态段中的偏移量，并没有什么用，我们只要关注.dynamic中出现了DT_BIND_NOW即表示符号在进程启动时就进行绑定了。

`DT_FLAGS`

```c++
#define DT_FLAGS	30		/* Flags for the object being loaded */

/* Values of `d_un.d_val' in the DT_FLAGS entry.  */
#define DF_ORIGIN	0x00000001	/* Object may use DF_ORIGIN */
#define DF_SYMBOLIC	0x00000002	/* Symbol resolutions starts here */
#define DF_TEXTREL	0x00000004	/* Object contains text relocations */
#define DF_BIND_NOW	0x00000008	/* No lazy binding for this object */
#define DF_STATIC_TLS	0x00000010	/* Module uses the static TLS model */
```

DT_FLAGS 是 ELF 文件中的一个动态段条目，它包含了一些标志位，用于指示程序的一些特性和特殊处理方式。DT_FLAGS 的值是一个位掩码，每个标志位的含义如下：

- DF_ORIGIN：指示动态链接器是否使用 $ORIGIN 环境变量来计算依赖库的路径。
- DF_SYMBOLIC：指示动态链接器是否符号共享。
- DF_BIND_NOW：指示程序是否在启动时立即进行符号绑定。
- DF_TEXTREL：指示程序中是否存在可写的代码段。
- DF_BIND_ORIGIN：指示动态链接器是否使用 $ORIGIN 环境变量来计算符号绑定的路径。
- DF_STATIC_TLS：指示程序是否使用静态 TLS 模型。

DF_BIND_NOW和DT_BIND_NOW作用相同，都表示是否使用了延迟绑定技术，所以判断一个程序是否使用了延迟绑定需要同时判定这两个字段。

## 二、libelf的使用

libelf是一个用于读取和编辑ELF（Executable and Linkable Format）文件的C语言库。ELF是一种用于可执行文件、共享库、目标文件等的文件格式，包括ELF32和ELF64两种格式。libelf库提供了一组API，用于读取、修改和生成ELF文件。

libelf库的API包括了以下功能：

- 打开、关闭和读取ELF文件的头信息。
- 读取和修改ELF文件的节表、符号表、重定位表等信息。
- 读取和修改ELF文件的程序头、节头等信息。
- 读取和修改ELF文件的符号、段、节等信息。
- 读取和修改ELF文件的动态链接信息。
- 读取和修改ELF文件的调试信息。

libelf库可以用于编写各种类型的工具，如ELF文件的解析器、修改器、重定位器、调试器等。libelf库的使用需要一定的C语言编程经验，但是它提供了很多方便的API，是ELF文件处理的重要工具之一。

libelf库的一个重要应用是动态链接器。动态链接器是一个负责在程序运行时加载和链接共享库的系统组件，它使用libelf库读取和解析ELF文件，以实现共享库的加载和链接。因此，libelf库在动态链接器的实现中扮演着重要的角色。——by ChatGpt

我没有找到更多的使用它的例子，主要部分在[6],[7]。关于libelf的使用，我会基于[7]中的教程《libelf-by-example》来展开（就是翻译啦，改几个字就是我的东西了）。

### 2.1 libelf的安装和简单使用

libelf是第三方库，需要安装才能使用：

```shell
sudo apt-get install libelf-dev 
```

之后，在代码中使用该库需要包含头文件：

```c++
#include <libelf.h>
```

编译时，需要指定libelf库：

```shell
g++ tracee.c -o tracee -lelf
```

下面是libelf的一个简单例子：

```c++
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
```



### 2.2 解析ELF Header

```c++
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
```





### 2.5 libelf常用API

#### 2.5.1 操作ELF文件

`elf_version`

```c++
unsigned int elf_version(unsigned int ver);
```

检查ELF库的版本号是否与期望的版本号匹配，但实际上ELF版本就没变过，当前版本（EV_CURRENT）为1。注意，在初始化一个ELF对象之前需要使用该函数指定版本号。

* ver：应用程序期望的版本号，通常使用常量（EV_CURRENT）

返回实际使用的ELF版本号。

`elf_kind`

```c++
int elf_kind(Elf *elf);
```

返回ELF文件的类型。

* elf：指向Elf类型的指针，指向一个已经打开的ELF对象

返回ELF文件的类型，返回值为ELF_K_NONE、ELF_K_AR、ELF_K_ELF、ELF_K_CORE、ELF_K_NUM之一。

`elf_begin`

```c++
Elf *elf_begin(int fd, Elf_Cmd cmd, Elf *ref);
```

从打开的ELF文件初始化一个ELF对象，返回该对象的指针，可以通过该指针访问ELF文件的所有信息

* fd：文件描述符，指向打开的ELF文件
* cmd：打开模式，通常为ELF_C_READ、ELF_C_WRITE、ELF_C_RDWR之一，需要和fd的打开模式相同
* ref：一个Elf结构体指针，用于返回打开的ELF文件的信息

返回一个Elf结构体指针

`elf_end`

```c++
int elf_end(Elf *elf);
```

关闭打开的ELF对象，并释放对应空间

* elf：指向Elf类型的指针，指向一个已经打开的ELF对象

返回0表示关闭成功，返回-1表示关闭失败。

`elf_getshdrstrndx`

```c++
int elf_getshdrstrndx(Elf *elf, size_t *shstrndx);
```

用于获取Section Header中strtab section的索引，索引通过shsrendx返回。

* elf：指向Elf类型的指针，指向一个已经打开的ELF对象
* shstrndx：指向size_t类型的指针，用于返回Section Header Table中strtab的索引

该函数返回一个int类型的值，表示函数的执行状态。如果函数执行成功，则返回0；否则返回一个非零值。

`elf_nextscn`

```c++
Elf_Scn *elf_nextscn(Elf *elf, Elf_Scn *scn);
```

用于遍历ELF中的section，第二参数指定当前section的Elf_Scn，第一次调用时置NULL。

* elf：指向Elf类型的指针，指向一个已经打开的ELF对象
* scn：指向Elf_Scn类型的指针，用于指定当前节的位置。在第一次调用该函数时，可以将该参数设置为NULL，以获取ELF文件中的第一个节。

该函数返回一个指向Elf_Scn类型的指针，该指针指向下一个节。如果已经到达ELF文件的末尾，则返回NULL。

`gelf_getshdr`

```c++
GElf_Shdr *gelf_getshdr(Elf_Scn *scn, GElf_Shdr *shdr);
```

获取指定section在Section Header Table中的描述信息

* scn：指向Elf_Scn类型的指针，该指针指向要获取节头信息的节
* shdr：指向GElf_Shdr类型的指针，用于存储节头信息

该函数返回一个指向GElf_Shdr类型的指针，该指针指向存储了节头信息的内存区域。

`elf_strptr`

```c++
char *elf_strptr(Elf *elf, size_t section_index, size_t offset);
```

用于获取strtab中指定索引的字符串

* elf：指向Elf类型的指针，指向一个已经打开的ELF对象
* section_index：一个size_t类型的值，表示strtab在Section Header Table中的索引，可以通过ELH Header或`elf_getshdrstrndx`获取。
* offset：一个size_t类型的值，表示要获取的字符串在字符串表中的偏移量。

该函数返回一个指向char类型的指针，该指针指向存储了指定字符串的内存区域。如果未找到指定字符串，则返回NULL。

`elf_ndxscn`

```c++
size_t elf_ndxscn(Elf_Scn *scn);
```

用于获取指定section在Section Header中的索引

* scn：指向Elf_Scn类型的指针，该指针指向要获取索引的节

该函数返回一个size_t类型的值，表示指定节的索引。

`elf_getscn`

```c++
Elf_Scn *elf_getscn(Elf *elf, size_t index);
```

用于获取指定索引的section

* elf：指向Elf类型的指针，指向一个已经打开的ELF对象
* index：一个size_t类型的值，表示要获取的节的索引。

#### 2.5.2 section表项查询

`gelf_getdyn`

```c++
/* Get information from dynamic table at the given index.  */
extern GElf_Dyn *gelf_getdyn (Elf_Data *__data, int __ndx, GElf_Dyn *__dst);
```

通过Elf_Data和索引来查询.dynamic section的表项。返回获取到的表项指针，查询失败返回NULL。

* data：是一个指向包含动态段条目的 Elf_Data 结构体的指针；
* ndx：要获取的动态段条目的索引；
* dst：一个指向 GElf_Dyn 结构体的指针，用于存储获取到的动态段条目；



## 参考引用

1. 《程序员的自我修养——链接、装载与库》
2. [Linux 下的 ELF 完整结构分析](https://www.yhspy.com/2020/06/17/Linux%20%E4%B8%8B%E7%9A%84%20ELF%20%E5%AE%8C%E6%95%B4%E7%BB%93%E6%9E%84%E5%88%86%E6%9E%90/)
3. [计算机那些事(4)——ELF文件结构](http://chuquan.me/2018/05/21/elf-introduce/)
4. [《ELF-64 Object File Format》](https://www.uclibc.org/docs/elf-64-gen.pdf)
5. [《Tool Interface Standard (TIS) Executable and Linking Format (ELF) Specification Version 1.2》](https://refspecs.linuxfoundation.org/elf/elf.pdf)
6. [《libelf库分析》](https://www.zybuluo.com/devilogic/note/139554)
7. [ELF Tool Chain Wiki](https://sourceforge.net/p/elftoolchain/wiki/libelf/)

