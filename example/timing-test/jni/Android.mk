LOCAL_PATH := $(call my-dir)
include $(LOCAL_PATH)/../../../android-ndk-profiler.mk

include $(CLEAR_VARS)

LOCAL_MODULE    := timing_jni
LOCAL_SRC_FILES := timing_jni.c dummy_calls.c
LOCAL_LDLIBS := -llog
# compile with profiling
LOCAL_CFLAGS := -pg
LOCAL_STATIC_LIBRARIES += andprof

include $(BUILD_SHARED_LIBRARY)
