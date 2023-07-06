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
#include "Helper.h"

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

void ForEachMapsSegment(FILE* mapsFile, std::function<bool(char*)> callback)
{
    char mapLine[256];

    while(fgets(mapLine, 256, mapsFile) != nullptr)
    {   
        if(callback((char*)mapLine) == false)
            break;
    }

    rewind(mapsFile);
}

void ForEachMapsSegmentContains(const char* str, FILE* mapsFile, std::function<bool(char*)> callback)
{
    ForEachMapsSegment(mapsFile, [&](char* segment){
        if(strstr(segment, str) == nullptr)
            return true;

        return callback(segment);
    });
}

uintptr_t FindModuleBaseAddress(int procId, const char* name, bool nb)
{
    uintptr_t result = INVALID_MODULE_BASE_ADDR;

    char mapsPath[256];

    memset(mapsPath, 0x0, 256);

    sprintf(mapsPath, "/proc/%d/maps", procId);

    OpenCallbackClose(mapsPath, "r", [&](FILE* f){
        result = FindModuleBaseAddress(f, name, nb);
    });

    return result;
}

uintptr_t FindModuleBaseAddress(FILE* mapsFile, const char* name, bool nb)
{
    uintptr_t result = INVALID_MODULE_BASE_ADDR;

    ForEachMapsSegmentContains(name, mapsFile, [&](char* mapLine) {

        if(nb != true)
        {
            if(strstr(mapLine, "/nb/") != nullptr)
                return true;
        }

        result = strtoll(strtok(mapLine, "-"), nullptr, 16);

        return false;

    });

    return result;
}

std::string FindModulePath(int procId, const char* name, bool nb)
{
    std::string path = "";

    char mapsPath[256];

    memset(mapsPath, 0x0, 256);

    sprintf(mapsPath, "/proc/%d/maps", procId);

    OpenCallbackClose(mapsPath, "r", [&](FILE* f){
        path = FindModulePath(f, name, nb);
    });

    return path;
}

std::string FindModulePath(FILE* mapsFile, const char* name, bool nb)
{
    std::string path = "";

    ForEachMapsSegmentContains(name, mapsFile, [&](char* mapLine) {

        if(nb != true)
        {
            if(strstr(mapLine, "/nb/") != nullptr)
                return true;
        }

        const char* pPath = strrchr(strtok(mapLine, "\n"), ' ');

        if(pPath == nullptr)
            return true;
        
        pPath++;
        path = std::string(pPath);

        return false;
    });

    return path;
}

uintptr_t FindModuleSymbol32(int procId, const char* moduleName, const char* symbolName, bool nb)
{
    std::string modulePath = FindModulePath(procId, moduleName, nb);

    if(modulePath.empty())
        return INVALID_SYMBOL_ADDR;

    uintptr_t moduleBase = FindModuleBaseAddress(procId, moduleName, nb);

    if(moduleBase == INVALID_MODULE_BASE_ADDR)
        return INVALID_SYMBOL_ADDR;

    size_t symbolOffset = GetSymbolOffset32(modulePath.c_str(), symbolName);

    if(symbolOffset == INVALID_SYMBOL_OFF)
        return INVALID_SYMBOL_ADDR;

    return moduleBase + symbolOffset;
}