#include "Helper.h"
#include <fstream>
#include <unistd.h>

bool FileExists(const char* pFileName)
{
    std::ifstream file(pFileName);

    return file.good();
}

bool ToAbsolutePath(const char* path, std::string& outPath)
{
    char absolutePath[PATH_MAX];

    const char* resolvedPath = realpath(path, absolutePath);

    outPath = resolvedPath == nullptr ? "" : std::string(resolvedPath);

    return resolvedPath != nullptr;
}