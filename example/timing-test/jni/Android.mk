LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# compile as arm code...
LOCAL_ARM_MODE := arm
LOCAL_MODULE    := timing_jni
LOCAL_SRC_FILES := timing_jni.c dummy_calls.cpp
# compile with profiling
LOCAL_CFLAGS := -pg
LOCAL_STATIC_LIBRARIES += android-ndk-profiler

include $(BUILD_SHARED_LIBRARY)
$(call import-module,android-ndk-profiler)
