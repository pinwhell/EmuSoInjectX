set moduleName=emuinj
set moduleName64=emuinj

nox_adb shell "su -c 'rm data/local/tmp/%moduleName%'"
nox_adb push libs/x86/%moduleName% data/local/tmp/%moduleName%
nox_adb shell "su -c 'chmod 7777 data/local/tmp/%moduleName%'"
nox_adb shell "su -c 'chown 0 data/local/tmp/%moduleName%'"
nox_adb shell "su -c 'chgrp 0 data/local/tmp/%moduleName%'"
