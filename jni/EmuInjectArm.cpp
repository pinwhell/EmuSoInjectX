#include "EmuInjectArm.h"
#include "PtraceRPCWrappers.h"
#include "Ptrace.h"
#include "Helper.h"
#include "Errors.h"
#include "LinuxProcess.h"
#include "RemoteString.h"
#include "dlfcn.h"

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

    bool bSucessdedInjection = false;
    
    bool result = PtraceStopCallbackResume(procId, [&]{
        RemoteString rs(procId, pLibAbsolutePath.c_str());

        bSucessdedInjection = PtraceCallNativeBridgeDlopen(procId, rs.mEntry, RTLD_NOW) != nullptr;

        if(bSucessdedInjection == false && IsLastErrorSet() == false)
            SetLastError(ERR_INJECTION_FAILED);
    });

    if(result == false)
        return false;

    if(bSucessdedInjection == false)
        return false;

    return true;
}