#pragma once

#include <string>
#include <stdio.h>
#include <functional>

bool FileExists(const char* pFileName);
bool ToAbsolutePath(const char* path, std::string& outPath);
bool OpenCallbackClose(const char* pFileName, const char* mode, std::function<void(FILE*)> callback);