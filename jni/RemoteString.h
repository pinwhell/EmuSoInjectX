#pragma once

#include <cstdint>

struct RemoteString {

    RemoteString(int procId, const char* str);
    ~RemoteString();

    uintptr_t mEntry;
    size_t mLen;
    int mProcId;
};