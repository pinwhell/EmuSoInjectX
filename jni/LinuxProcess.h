#pragma once

#include <cstdio>
#include <vector>
#include <string>

#define INVALID_PROCESS_ID (~((int)0))
#define INVALID_MODULE_BASE_ADDR (~((unsigned int)0))
#define INVALID_SYMBOL_ADDR (~((unsigned int)0))
#define INVALID_HANDLE_VALUE (void*)(~((int)0))
#define HANDLE_VALID(handle) ((handle < 0) == false)

typedef void* HANDLE;

struct Handle {
    int mPid;
    int mMemFD;
    FILE* mMapsFile;
};

std::vector<int> getAllThreadIds(HANDLE hProc);
bool CastHandle(HANDLE hProc, Handle** outHandle);
int FindProcessId(const char* processName);
HANDLE OpenProcess(int pid);
void CloseProcess(HANDLE handle);
bool ReadProcessMemory(HANDLE hProc, unsigned int addr, void* data, size_t len);
bool WriteProcessMemory(HANDLE hProc, unsigned int addr, const void* data, size_t len);
uintptr_t FindModuleBaseAddress(HANDLE hProc, const char* name, bool nb = false);
std::string FindModulePath(HANDLE hProc, const char* name, bool nb = false);
uintptr_t FindModuleSymbol32(HANDLE hProc, const char* moduleName, const char* symbolName, bool nb = false);

bool PushSnapshot(HANDLE handle, unsigned int atAddr, size_t len);
bool PopSnapshot(HANDLE handle, unsigned int atAddr);


template<typename T>
T ReadProcessMemoryWrapper(HANDLE hProc, unsigned int addr)
{
    T obj;

    ReadProcessMemory(hProc, addr, (void*)&obj, sizeof(T));

    return obj;
}

template<typename T>
bool WriteProcessMemoryWrapper(HANDLE hProc, unsigned int addr, const T& obj)
{
    return WriteProcessMemory(hProc, addr, (const void*)&obj, sizeof(T));
}