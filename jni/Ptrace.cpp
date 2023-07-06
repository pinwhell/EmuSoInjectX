#include "Ptrace.h"
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <vector>
#include <unordered_map>
#include <stack>
#include "LinuxProcess.h"
#include "Errors.h"

template<typename T, typename K>
long Ptrace(long request, unsigned long pid, T addr, K data)
{
    long result = 0;

    if ((result = ptrace(request, pid, (void*)addr, (void*)data)) < 0)
    {
        SetLastError(ERR_ACCESS_DENIED);
        return result;
    }

    return result;
}

bool PtraceStopCallbackResume(int procId, std::function<void()> callback)
{
    if (Ptrace(PTRACE_ATTACH, procId, NULL, NULL) < 0) 
        return false;

    int status;

    waitpid(procId, &status, 0);

    callback();

    if (Ptrace(PTRACE_DETACH, procId, NULL, NULL) < 0)
        return false;

    return true;
}

bool GetContext(int procId, user_regs_struct& ctx)
{
    if(Ptrace(PTRACE_GETREGS, procId, NULL, &ctx) < 0)
        return false;

    return true;
}

bool SetContext(int procId, user_regs_struct& ctx)
{
    if(Ptrace(PTRACE_SETREGS, procId, NULL, &ctx) < 0)
        return false;

    return true;
}

bool PtraceContinue(int procId)
{
    if(Ptrace(PTRACE_CONT, procId, NULL, NULL) < 0)
        return false;

    return true;
}

uintptr_t PtraceCall(int procId, uintptr_t entry, const std::vector<uint32_t>& params)
{
    PushContext(procId);

    struct user_regs_struct ctx;

    if(GetContext(procId, ctx) == false)
        return -1;

    ctx.eip = entry;

    size_t spaceForParams = params.size() * sizeof(uint32_t);

    // Writing the params
    ctx.esp -= spaceForParams;
    PtraceWriteProcessMemory(procId, ctx.esp, params.data(), spaceForParams);

    // Writing the Ret Address ( zero to cause trouble =), so we get notified at call completion )
    ctx.esp -= sizeof(uint32_t);
    PtraceWriteProcessMemoryWrapper<uint32_t>(procId, ctx.esp, 0x0);

    SetContext(procId, ctx);

    // Now lets just run and wait
    if(PtraceContinue(procId) == false)
        return -1;

    int status;

    waitpid(procId, &status, WUNTRACED);  

    if(GetContext(procId, ctx) == false)
        return -1;

    if(PopContext(procId) == false)
        return -1;

    return ctx.eax;
}

uintptr_t PtraceCallModuleSymbol(int procId, const char* module, const char* symbol, bool nb, const std::vector<uint32_t>& params)
{
    uintptr_t symbolEntry = FindModuleSymbol32(procId, module, symbol, nb);

    if(symbolEntry == INVALID_SYMBOL_ADDR)
    {
        SetLastError(ERR_SYMBOL_NOT_FOUND);
        return 0;
    }

    return PtraceCall(procId, symbolEntry, params);
}

bool PtraceReadProcessMemory(int pid, unsigned int addr, void* data, size_t len) {
    uint32_t i, j, remain;    
    uint8_t *laddr;    
    
    union u {    
        long val;    
        char chars[sizeof(long)];    
    } d;    
    
    j = len / 4;    
    remain = len % 4;    
    
    laddr = (uint8_t *)data;    
    
    for (i = 0; i < j; i ++) {    
        d.val = Ptrace(PTRACE_PEEKTEXT, pid, addr, 0);    
        memcpy(laddr, d.chars, 4);    
        addr += 4;    
        laddr += 4;    
    }    
    
    if (remain > 0) {    
        d.val = Ptrace(PTRACE_PEEKTEXT, pid, addr, 0);    
        memcpy(laddr, d.chars, remain);    
    }    
    
    return true;
}

bool PtraceWriteProcessMemory(int pid, unsigned int addr, const void* data, size_t len) {
    uint32_t i, j, remain;    
    uint8_t *laddr;    
    
    union u {    
        long val;    
        char chars[sizeof(long)];    
    } d;    
    
    j = len / 4;    
    remain = len % 4;    
    
    laddr = (uint8_t *)data;    
    
    for (i = 0; i < j; i ++) {    
        memcpy(d.chars, laddr, 4);    
        Ptrace(PTRACE_POKETEXT, pid, addr, d.val);    
    
        addr  += 4;    
        laddr += 4;    
    }    
    
    if (remain > 0) {    
        d.val = Ptrace(PTRACE_PEEKTEXT, pid, addr, 0);    
        for (i = 0; i < remain; i ++) {    
            d.chars[i] = *laddr ++;    
        }    
    
        Ptrace(PTRACE_POKETEXT, pid, addr, d.val);    
    }    
    
    return true;    
}


std::unordered_map<int, std::unordered_map<unsigned int, std::stack<std::vector<unsigned char>>>> snapshots;

bool PtracePushSnapshot(int procId, unsigned int atAddr, size_t len)
{
    auto& snapshotStack = snapshots[procId][atAddr];
    snapshotStack.push(std::vector<unsigned char>(len));

    if (!PtraceReadProcessMemory(procId, atAddr, snapshotStack.top().data(), len)) {
        snapshotStack.pop();
        if (snapshotStack.empty())
            snapshots[procId].erase(atAddr);
        return false;
    }

    return true;
}

bool PtracePopSnapshot(int procId, unsigned int atAddr)
{
    auto pidIt = snapshots.find(procId);
    if (pidIt == snapshots.end())
        return false;

    auto addrIt = pidIt->second.find(atAddr);
    if (addrIt == pidIt->second.end())
        return false;

    auto& snapshotStack = addrIt->second;
    if (snapshotStack.empty())
        return false;

    if (!PtraceWriteProcessMemory(procId, atAddr, snapshotStack.top().data(), snapshotStack.top().size())) {
        return false;
    }

    snapshotStack.pop();
    if (snapshotStack.empty())
        pidIt->second.erase(addrIt);

    return true;
}

std::unordered_map<int, std::stack< user_regs_struct >> contexts;

bool PushContext(int procId) {
    user_regs_struct ctx;
    if (!GetContext(procId, ctx))
        return false;

    contexts[procId].push(ctx);

    return true;
}

bool PopContext(int procId) {
    auto stackIt = contexts.find(procId);
    if (stackIt == contexts.end())
        return false;

    std::stack<user_regs_struct>& stack = stackIt->second;

    if (stack.empty())
        return false;

    if (!SetContext(procId, stack.top()))
        return false;

    stack.pop();

    if (stack.empty())
        contexts.erase(stackIt);

    return true;
}
