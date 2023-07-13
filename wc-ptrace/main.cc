#include "wc_elf.h"

int
main(int argc, char **argv)
{
    if (argc != 2)
        errx(EXIT_FAILURE, "usage: %s file-name", argv[0]);

    std::string path(argv[1]);
    wc_elf::TraceeElf object(path);
    object.initialize();
    return 0;
}