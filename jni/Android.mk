LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_LDLIBS := -llog
LOCAL_ARM_NEON := false

LOCAL_MODULE := emuinj

LOCAL_SRC_FILES := \
main.cpp \
EmuInjectArm.cpp \
LinuxProcess.cpp \
Errors.cpp \
Helper.cpp \
Ptrace.cpp \
ELFHelper.cpp \
RemoteString.cpp \
PtraceRPCWrappers.cpp

include $(BUILD_EXECUTABLE)