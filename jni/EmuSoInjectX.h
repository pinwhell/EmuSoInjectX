#pragma once

enum ESIXErr {
    ERR_OK,
    ERR_PROCESS_NOT_FOUND,
    ERR_INJECTION_FILE_NOT_FOUND,
    ERR_ACCESS_DENIED,
    ERR_INJECTION_FAILED
};

/*Set the Error State*/
void SetLastError(ESIXErr err);

/*Get and Flush the Last Error*/
ESIXErr GetLastError();

const char* FormatError(ESIXErr err);
const char* FormatLastError();