#include "ELFHelper.h"
#include "Helper.h"
#include <ELF.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>

size_t GetSymbolOffset32(const char* elfPath, const char* symbolName)
{
    size_t result = INVALID_SYMBOL_OFF;

    if(FileExists(elfPath) == false)
        return result;

    int fd = open(elfPath, O_RDONLY);
    
    if(fd < 0)
        return result;

    union {
        void* entryRaw;
        uintptr_t entryAddr;
        Elf32_Ehdr* entryElf;
    };

    struct stat elfStat;

    if(fstat(fd, &elfStat) < 0)
        return result;
    
    entryRaw = mmap(NULL, elfStat.st_size, PROT_READ, MAP_SHARED, fd, 0 );

    if(entryRaw == MAP_FAILED)
        return result;

    Elf32_Shdr* sections = (Elf32_Shdr*)((Elf32_Off)entryAddr + entryElf ->e_shoff);

    Elf32_Shdr* symtab = nullptr;

    for(int i = 0; i < entryElf ->e_shnum; i++)
    {
        if(sections[i].sh_type != SHT_SYMTAB)
            continue;

        symtab = sections + i;
        break;
    }

    // Symbol table not found
    if(symtab == nullptr)
        return result;

    union {
        uintptr_t strSecAddr;
        const char* strSecAddrStr;
    };

    strSecAddr = entryAddr + sections[symtab->sh_link].sh_offset;
    Elf32_Sym* symSec = (Elf32_Sym*)(entryAddr + symtab->sh_offset);
    int nSymbols = symtab->sh_size / sizeof(Elf32_Sym);

    for(int i = 0; i < nSymbols;  i++)
    {
        if((ELF32_ST_BIND(symSec[i].st_info) & (STT_FUNC | STB_GLOBAL)) == false)
            continue;

        const char* currSymbolName = strSecAddrStr + symSec[i].st_name;

        if(strcmp(currSymbolName, symbolName) != 0)
            continue;
            
        result = symSec[i].st_value;
        break;
    }

    munmap(entryRaw, elfStat.st_size);

    close(fd);

    return result;
}