LOCAL_PATH := $(call my-dir)
# remove -ffunction-sections and -fomit-frame-pointer from the default compile flags
TARGET_thumb_release_CFLAGS := $(filter-out -ffunction-sections,$(TARGET_thumb_release_CFLAGS))
TARGET_thumb_release_CFLAGS := $(filter-out -fomit-frame-pointer,$(TARGET_thumb_release_CFLAGS))
TARGET_CFLAGS := $(filter-out -ffunction-sections,$(TARGET_CFLAGS))

# include libandprof.a in the build
include $(CLEAR_VARS)
LOCAL_MODULE := andprof
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libandprof.a
include $(PREBUILT_STATIC_LIBRARY)


include $(CLEAR_VARS)

LOCAL_MODULE    := timing_jni
LOCAL_SRC_FILES := timing_jni.c dummy_calls.c
LOCAL_LDLIBS := -llog
# compile with profiling
LOCAL_CFLAGS := -pg
LOCAL_STATIC_LIBRARIES += andprof

include $(BUILD_SHARED_LIBRARY)
