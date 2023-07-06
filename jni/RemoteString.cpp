#include "RemoteString.h"
#include "PtraceRPCWrappers.h"
#include <string.h>
#include "Ptrace.h"
#include <sys/mman.h>

RemoteString::RemoteString(int procId, const char* str)
    : mProcId(procId)
{
    mLen = strlen(str);

    mEntry = PtraceCallMMap(mProcId, mLen, PROT_READ);

    if(mEntry < 0 == false)
    {
        PtraceWriteProcessMemory(mProcId, mEntry, str, mLen);
        return;
    }

    mEntry = 0;
}

RemoteString::~RemoteString()
{
    if(mEntry)
    {
        PtraceCallMUnmap(mProcId, mEntry, mLen);
    }
}
