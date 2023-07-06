#pragma once

#include <functional>

#include <sys/user.h>

bool PtraceStopCallbackResume(int procId, std::function<void()> callback);

bool SetContext(int procId, user_regs_struct& ctx);
bool GetContext(int procId, user_regs_struct& ctx);

bool PtraceReadProcessMemory(int pid, unsigned int addr, void* data, size_t len);
bool PtraceWriteProcessMemory(int pid, unsigned int addr, const void* data, size_t len);

bool PtracePushSnapshot(int procId, unsigned int atAddr, size_t len);
bool PtracePopSnapshot(int procId, unsigned int atAddr);

bool PushContext(int procId);
bool PopContext(int procId);

bool PtraceContinue(int procId);

/*This function spect a context of ptrace alredy attached and the process alredy stopped*/
uintptr_t PtraceCall(int procId, uintptr_t entry, const std::vector<uint32_t>& params);
uintptr_t PtraceCallModuleSymbol(int procId, const char* module, const char* symbol, bool nb, const std::vector<uint32_t>& params);

/* Wrappers */
template<typename T>
T PtraceReadProcessMemoryWrapper(int procId, unsigned int addr)
{
    T obj;

    PtraceReadProcessMemory(procId, addr, (void*)&obj, sizeof(T));

    return obj;
}

template<typename T>
bool PtraceWriteProcessMemoryWrapper(int procId, unsigned int addr, const T& obj)
{
    return PtraceWriteProcessMemory(procId, addr, (const void*)&obj, sizeof(T));
}