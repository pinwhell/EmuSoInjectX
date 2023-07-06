#pragma once

#include <cstdint>

uintptr_t PtraceCallMMap(int procId, size_t size, int prot);
void* PtraceCallNativeBridgeDlopen(int procId, uintptr_t libPathRemoteAddr, int mode);
void PtraceCallMUnmap(int procId, uintptr_t entry, size_t size);

