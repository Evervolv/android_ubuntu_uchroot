LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := uchroot
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES:= uchroot.c
LOCAL_STATIC_LIBRARIES := libc
include $(BUILD_EXECUTABLE)
