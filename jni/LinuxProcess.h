#pragma once

#include <cstdio>
#include <vector>
#include <string>
#include<functional>

#define INVALID_PROCESS_ID (~((int)0))
#define INVALID_MODULE_BASE_ADDR (~((unsigned int)0))
#define INVALID_SYMBOL_ADDR (~((unsigned int)0))

int FindProcessId(const char* processName);

void ForEachMapsSegment(FILE* mapsFile, std::function<bool(const char*)> callback);
void ForEachMapsSegmentContains(const char* str, FILE* mapsFile, std::function<bool(char*)> callback);

uintptr_t FindModuleBaseAddress(FILE* mapsFile, const char* name, bool nb = false);
uintptr_t FindModuleBaseAddress(int procId, const char* name, bool nb = false);

std::string FindModulePath(FILE* mapsFile, const char* name, bool nb = false);
std::string FindModulePath(int procId, const char* name, bool nb = false);

uintptr_t FindModuleSymbol32(int procId, const char* moduleName, const char* symbolName, bool nb = false);