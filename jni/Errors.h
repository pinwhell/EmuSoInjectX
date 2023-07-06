#pragma once

enum Err {
    ERR_OK,
    ERR_PROCESS_NOT_FOUND,
    ERR_INJECTION_FILE_NOT_FOUND,
    ERR_ACCESS_DENIED,
    ERR_INJECTION_FAILED,
    ERR_SYMBOL_NOT_FOUND
};

/*Set the Error State*/
void SetLastError(Err err);

/*Get and Flush the Last Error*/
Err GetLastError();

const char* FormatError(Err err);
const char* FormatLastError();

bool IsLastErrorSet();