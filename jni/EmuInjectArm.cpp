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
#include <dlfcn.h>

#define NATIVEBRIDGE_LOADLIB_SYMNAME "_ZN7android23NativeBridgeLoadLibraryEPKci"

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

void* PtraceCallNativeBridgeDlopen(HANDLE hProc, uintptr_t libPathRemoteAddr, int mode)
{
    Handle* pHandle = nullptr;

    if(CastHandle(hProc, &pHandle) == false)
        return nullptr;

    uintptr_t nbDlopen = FindModuleSymbol32(hProc, "libnativebridge.so", NATIVEBRIDGE_LOADLIB_SYMNAME);

    if(nbDlopen == INVALID_SYMBOL_ADDR)
        return nullptr;

    return (void*)PtraceCall(pHandle->mPid, nbDlopen, {
        libPathRemoteAddr,
        (unsigned int)mode
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

struct RemoteString {

    RemoteString(HANDLE hProc, const char* str)
        : mhProc(hProc)
    {
        Handle* pHandle = nullptr;

        if(CastHandle(hProc, &pHandle) == false)
            return;

        mLen = strlen(str);

        mEntry = PtraceCallMMap(mhProc, mLen, PROT_READ);

        if(mEntry < 0 == false)
        {
            PtraceWriteProcessMemory(pHandle->mPid, mEntry, str, mLen);
            return;
        }

        mEntry = 0;
    }

    ~RemoteString()
    {
        if(mEntry)
        {
            PtraceCallMUnmap(mhProc, mEntry, mLen);
        }
    }

    uintptr_t mEntry;
    size_t mLen;
    HANDLE mhProc;
};

bool EmuInjectArm::Inject(const char* pProcName, const char* pLibPath)
{
    if(FileExists(pLibPath) == false)
    {
        SetLastError(ERR_INJECTION_FILE_NOT_FOUND);
        return false;
    }

    std::string pLibAbsolutePath = "";

    if(ToAbsolutePath(pLibPath, pLibAbsolutePath) == false)
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
    bool bSucessdedInjection = false;
    bool result = _PtraceStopCallbackResume(procId, [&]{
        RemoteString rs(hProc, pLibAbsolutePath.c_str());

        bSucessdedInjection = PtraceCallNativeBridgeDlopen(hProc, rs.mEntry, RTLD_NOW) != nullptr;

        if(bSucessdedInjection == false)
            SetLastError(ERR_INJECTION_FAILED);
    });

    CloseProcess(hProc);

    if(result == false)
        return false;

    if(bSucessdedInjection == false)
        return false;

    return true;
}