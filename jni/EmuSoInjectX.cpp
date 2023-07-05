#include "EmuSoInjectX.h"
#include <unordered_map>
#include <string>

ESIXErr gLastError = ERR_OK;

std::unordered_map<ESIXErr, std::string> gErrStr {
    {ERR_OK, "Error Ok"},
    {ERR_PROCESS_NOT_FOUND, "Process Not Found"},
    {ERR_INJECTION_FILE_NOT_FOUND, "Injection File Not Found"},
};

void SetLastError(ESIXErr err)
{
    gLastError = err;
}

ESIXErr GetLastError()
{
    ESIXErr outErr = gLastError;

    gLastError = ERR_OK;

    return outErr;
}

const char* FormatError(ESIXErr err)
{
    const std::string& strRef = gErrStr[err];

    return strRef.c_str();
}

const char* FormatLastError()
{
    return FormatError(GetLastError());
}