#include <EmuInjectArm.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#include "LinuxProcess.h"
#include "EmuSoInjectX.h"
#include "Helper.h"
#include "Ptrace.h"
#include "ELFHelper.h"

// Must Remove
#include <stdio.h>

bool _PtraceStopCallbackResume(int procId, std::function<void()> callback)
{
    bool result = PtraceStopCallbackResume(procId, callback);

    if(result == false)
        SetLastError(ERR_ACCESS_DENIED);

    return result;
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

    size_t malloc = FindModuleSymbol32(hProc, "libc.so", "malloc");

    bool result = _PtraceStopCallbackResume(procId, [&]{

        auto result = PtraceCall(procId, malloc, {0x1000});
        printf("Malloc Returned %08X\n", result);

    });

    if(result == false)
        return false;

    CloseProcess(hProc);

    return true;
}