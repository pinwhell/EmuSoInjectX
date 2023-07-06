#include "PtraceRPCWrappers.h"
#include "Ptrace.h"
#include <sys/mman.h>
#include <vector>

#define NATIVEBRIDGE_LOADLIB_SYMNAME "_ZN7android23NativeBridgeLoadLibraryEPKci"

uintptr_t PtraceCallMMap(int procId, size_t size, int prot)
{
    return PtraceCallModuleSymbol(procId, "libc.so", "mmap", false, {
        0x0,
        size,
        (unsigned int)prot,
        MAP_PRIVATE|MAP_ANONYMOUS,
        (unsigned int)-1,
        0
    });
}

void* PtraceCallNativeBridgeDlopen(int procId, uintptr_t libPathRemoteAddr, int mode)
{
    return (void*)PtraceCallModuleSymbol(procId, "libnativebridge.so", NATIVEBRIDGE_LOADLIB_SYMNAME, false, {
        libPathRemoteAddr,
        (unsigned int)mode
    });
}

void PtraceCallMUnmap(int procId, uintptr_t entry, size_t size)
{
    PtraceCallModuleSymbol(procId, "libc.so", "munmap",  false, {
        entry,
        size
    });
}