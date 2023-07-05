#include <LinuxProcess.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

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