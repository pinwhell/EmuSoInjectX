#pragma once

class EmuInjectArm 
{
    public:
    static bool Inject(const char* pProcName, const char* pLibPath);
};