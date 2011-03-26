LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := andprof
LOCAL_SRC_FILES := gnu_mcount.S prof.c read_smaps.c
include $(BUILD_STATIC_LIBRARY)
