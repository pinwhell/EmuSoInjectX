#include <iostream>

int main(int argv, const char** argc)
{
    if(argv < 2)
    {
        printf("./emuinj [path to library]");
        return 1;
    }

    printf("%s\n", argc[1]);

    //EmuInjectArm::Inject();
}