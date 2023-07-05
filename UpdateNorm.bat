set moduleName=emuinj

adb shell "su -c 'rm data/local/tmp/%moduleName%'"
adb push libs/x86/%moduleName% data/local/tmp/%moduleName%
adb shell "su -c 'chmod 7777 data/local/tmp/%moduleName%'"
adb shell "su -c 'chown 0 data/local/tmp/%moduleName%'"
adb shell "su -c 'chgrp 0 data/local/tmp/%moduleName%'"
