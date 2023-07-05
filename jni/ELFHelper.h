#pragma once

#include <cstdint>

#define INVALID_SYMBOL_OFF (~(size_t)0)

size_t GetSymbolOffset32(const char* elfPath, const char* symbolName);