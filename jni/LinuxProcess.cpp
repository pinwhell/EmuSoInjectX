#include "LinuxProcess.h"
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <unordered_map>
#include <stack>
#include <memory>
#include <vector>
#include <string.h>
#include <ELFHelper.h>

bool CastHandle(HANDLE hProc, Handle** outHandle)
{
    if(hProc == INVALID_HANDLE_VALUE)
        return false;

    if(hProc == nullptr)
        return false;

    *outHandle = (Handle*)hProc;

    return true;
}

int FindProcessId(const char* processName)
{
    if(processName == nullptr)
        return INVALID_PROCESS_ID;

    DIR * pDir = opendir("/proc/");

    if(pDir == nullptr)
        return INVALID_PROCESS_ID;

    int procId = INVALID_PROCESS_ID;

    while(struct dirent* pDChild = readdir(pDir))
    {
        int currPid = atoi(pDChild->d_name);

        if(currPid == 0)
            continue;

        char cmdline[256];
        char cmdlinePath[256];

        memset(cmdline, 0x0, 256);
        memset(cmdlinePath, 0x0, 256);

        sprintf(cmdlinePath, "/proc/%d/cmdline", currPid);

        FILE* cmdlineFile = fopen(cmdlinePath, "r");

        if(cmdlineFile == nullptr)
            continue;

        if(fgets(cmdline, 256, cmdlineFile) != nullptr)
        {
            if(strcmp(cmdline, processName) != 0)
                goto cnt_close_cmdln;

            fclose(cmdlineFile);
            procId = currPid;
            break;
        }

        cnt_close_cmdln:
        fclose(cmdlineFile);
    }

    closedir(pDir);

    return procId;
}

HANDLE OpenProcess(int pid)
{
    if(pid == INVALID_PROCESS_ID)
        return INVALID_HANDLE_VALUE;

    Handle* handle = new Handle();

    handle->mPid = pid;

    char memPath[256];
    char mapsPath[256];

    memset(memPath, 0x0, 256);
    memset(mapsPath, 0x0, 256);

    sprintf(memPath, "/proc/%d/mem", pid);
    sprintf(mapsPath, "/proc/%d/maps", pid);

    handle->mMemFD = open(memPath, O_RDWR);

    if(handle->mMemFD < 0)
        goto ret_fail;

    handle->mMapsFile = fopen(mapsPath, "r");

    if (handle->mMapsFile == NULL)
        goto ret_fail;

    return (HANDLE)handle;

    ret_fail:
    delete handle;
    return INVALID_HANDLE_VALUE; 
}

void CloseProcess(HANDLE _handle)
{
    Handle* pHandle = nullptr;

    if(CastHandle(_handle, &pHandle) == false)
        return;

    fclose(pHandle->mMapsFile);
    close(pHandle->mMemFD);

    delete pHandle;    
}

bool ReadProcessMemory(HANDLE hProc, unsigned int addr, void* data, size_t len)
{
    Handle* pHandle = nullptr;

    if(CastHandle(hProc, &pHandle) == false)
        return false;

    if(lseek(pHandle->mMemFD, addr, SEEK_SET) < 0)
        return false;

    if(read(pHandle->mMemFD, data, len) < 0)
        return false;

    return true;
}

bool WriteProcessMemory(HANDLE hProc, unsigned int addr, const void* data, size_t len)
{
    Handle* pHandle = nullptr;

    if(CastHandle(hProc, &pHandle) == false)
        return false;

    if(lseek(pHandle->mMemFD, addr, SEEK_SET) < 0)
        return false;

    if(write(pHandle->mMemFD, data, len) < 0)
        return false;

    return true;
}

uintptr_t FindModuleBaseAddress(HANDLE hProc, const char* name, bool nb)
{
    Handle* pHandle = nullptr;
    uintptr_t result = INVALID_MODULE_BASE_ADDR;

    if(CastHandle(hProc, &pHandle) == false)
        return result;

    char mapLine[256];

    while(fgets(mapLine, 256, pHandle->mMapsFile) != nullptr)
    {   
        if(strstr(mapLine, name) == nullptr)
            continue;

        if(nb != true)
        {
            if(strstr(mapLine, "/nb/") != nullptr)
                continue;
        }

        result = strtoll(strtok(mapLine, "-"), nullptr, 16);

        break;
    }

    rewind(pHandle->mMapsFile);

    return result;
}

std::string FindModulePath(HANDLE hProc, const char* name, bool nb)
{
    Handle* pHandle = nullptr;
    std::string path = "";

    if(CastHandle(hProc, &pHandle) == false)
        return path;

    char mapLine[256];

    while(fgets(mapLine, 256, pHandle->mMapsFile) != nullptr)
    {
        if(strstr(mapLine, name) == nullptr)
            continue;

        if(nb != true)
        {
            if(strstr(mapLine, "/nb/") != nullptr)
                continue;
        }

        const char* pPath = strrchr(strtok(mapLine, "\n"), ' ');

        if(pPath == nullptr)
            continue;
        
        pPath++;
        path = std::string(pPath);

        break;
    }

    rewind(pHandle->mMapsFile);

    return path;
}

uintptr_t FindModuleSymbol32(HANDLE hProc, const char* moduleName, const char* symbolName, bool nb)
{
    std::string modulePath = FindModulePath(hProc, moduleName, nb);

    if(modulePath.empty())
        return INVALID_SYMBOL_ADDR;

    uintptr_t moduleBase = FindModuleBaseAddress(hProc, moduleName, nb);

    if(moduleBase == INVALID_MODULE_BASE_ADDR)
        return INVALID_SYMBOL_ADDR;

    size_t symbolOffset = GetSymbolOffset32(modulePath.c_str(), symbolName);

    if(symbolOffset == INVALID_SYMBOL_OFF)
        return INVALID_SYMBOL_ADDR;

    return moduleBase + symbolOffset;
}