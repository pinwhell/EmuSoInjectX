#include "Errors.h"
#include <unordered_map>
#include <string>

Err gLastError = ERR_OK;

std::unordered_map<Err, std::string> gErrStr {
    {ERR_OK, "Error Ok"},
    {ERR_PROCESS_NOT_FOUND, "Process Not Found"},
    {ERR_INJECTION_FILE_NOT_FOUND, "Injection File Not Found"},
    {ERR_ACCESS_DENIED, "Access Denied"},
    {ERR_INJECTION_FAILED, "Injection Failed"},
    {ERR_SYMBOL_NOT_FOUND, "Symbol Not Found"}
};

void SetLastError(Err err)
{
    gLastError = err;
}

Err GetLastError()
{
    Err outErr = gLastError;

    gLastError = ERR_OK;

    return outErr;
}

const char* FormatError(Err err)
{
    const std::string& strRef = gErrStr[err];

    return strRef.c_str();
}

const char* FormatLastError()
{
    return FormatError(GetLastError());
}

bool IsLastErrorSet()
{
    return gLastError != ERR_OK;
}