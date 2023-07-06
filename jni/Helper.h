#pragma once

#include <string>

bool FileExists(const char* pFileName);
bool ToAbsolutePath(const char* path, std::string& outPath);