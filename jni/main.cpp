#include <iostream>
#include <EmuInjectArm.h>

int main(int argv, const char** argc)
{
    if(argv < 2)
    {
        printf("./emuinj [path to library]\n");
        return 1;
    }

    EmuInjectArm::Inject(argc[1]);
}