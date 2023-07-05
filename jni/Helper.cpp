#include "Helper.h"
#include <fstream>

bool FileExists(const char* pFileName)
{
    std::ifstream file(pFileName);
    
    return file.good();
}