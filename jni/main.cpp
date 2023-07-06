#include <iostream>
#include <EmuInjectArm.h>
#include "Errors.h"

void PrintHelp()
{
   printf("./emuinj [process name] [path to library]\n"); 
}

int main(int argv, const char** argc)
{
    if(argv != 3)
    {
        PrintHelp();
        return 1;
    }

    if(EmuInjectArm::Inject(argc[1], argc[2]) == false)
    {
        printf("Error: %s\n", FormatLastError());
        return 1;
    }

    printf("Injected Sucessfully\n");

    return 0;
}