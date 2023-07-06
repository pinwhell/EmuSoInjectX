#include <EmuInjectArm.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#include "LinuxProcess.h"
#include "EmuSoInjectX.h"
#include "Helper.h"
#include "Ptrace.h"
#include "ELFHelper.h"
#include <sys/mman.h>

// Must Remove
#include <stdio.h>

bool _PtraceStopCallbackResume(int procId, std::function<void()> callback)
{
    bool result = PtraceStopCallbackResume(procId, callback);

    if(result == false)
        SetLastError(ERR_ACCESS_DENIED);

    return result;
}

uintptr_t PtraceCallMMap(HANDLE hProc, size_t size, int prot)
{
    Handle* pHandle = nullptr;

    if(CastHandle(hProc, &pHandle) == false)
        return -1;

    uintptr_t mmap = FindModuleSymbol32(hProc, "libc.so", "mmap");

    if(mmap == INVALID_SYMBOL_ADDR)
        return -1;

    return PtraceCall(pHandle->mPid, mmap, {
        0x0,
        size,
        (unsigned int)prot,
        MAP_PRIVATE|MAP_ANONYMOUS,
        (unsigned int)-1,
        0
    });
}

uintptr_t PtraceCallMUnmap(HANDLE hProc, uintptr_t entry, size_t size)
{
    Handle* pHandle = nullptr;

    if(CastHandle(hProc, &pHandle) == false)
        return -1;

    uintptr_t munmap = FindModuleSymbol32(hProc, "libc.so", "munmap");

    if(munmap == INVALID_SYMBOL_ADDR)
        return -1;

    return PtraceCall(pHandle->mPid, munmap, {
        entry,
        size
    });
}

bool EmuInjectArm::Inject(const char* pProcName, const char* pLibPath)
{
    if(FileExists(pLibPath) == false)
    {
        SetLastError(ERR_INJECTION_FILE_NOT_FOUND);
        return false;
    }

    int procId = FindProcessId(pProcName);

    if(procId == INVALID_PROCESS_ID)
    {
        SetLastError(ERR_PROCESS_NOT_FOUND);
        return false;
    }

    HANDLE hProc = OpenProcess(procId);

    if(hProc == INVALID_HANDLE_VALUE)
    {
        SetLastError(ERR_ACCESS_DENIED);
        return false;
    }

    

    bool result = _PtraceStopCallbackResume(procId, [&]{

        uintptr_t result = PtraceCallMMap(hProc, 0x1000, PROT_READ | PROT_WRITE);
        printf("Mmap Returned %08X\n", result);

        getchar();
        getchar();

        PtraceCallMUnmap(hProc, result, 0x1000);
    });

    if(result == false)
        return false;

    CloseProcess(hProc);

    return true;
}