#include <EmuInjectArm.h>
#include <sys/ptrace.h>
#include "LinuxProcess.h"
#include "EmuSoInjectX.h"
#include "Helper.h"

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
    

    return true;
}