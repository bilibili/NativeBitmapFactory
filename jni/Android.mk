LOCAL_PATH :=$(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := libskia.$(API)
LOCAL_SRC_FILES := ../AndroidLibs/$(TARGET_ARCH)/libskia.$(API).so

ifeq ($(TARGET_ARCH),x86)
ifneq ($(API),19)
ifneq ($(API),19-2)
LOCAL_SRC_FILES := ../AndroidLibs/$(TARGET_ARCH)/libskia.17.so
endif
endif
else ifeq ($(TARGET_ARCH),mips)
LOCAL_SRC_FILES := ../AndroidLibs/$(TARGET_ARCH)/libskia.17.so
endif
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libandroid_runtime.$(API)
LOCAL_SRC_FILES := ../AndroidLibs/$(TARGET_ARCH)/libandroid_runtime.$(API).so
ifeq ($(TARGET_ARCH),x86)
ifneq ($(API),19)
LOCAL_SRC_FILES := ../AndroidLibs/$(TARGET_ARCH)/libandroid_runtime.17.so
endif
else ifeq ($(TARGET_ARCH),mips)
LOCAL_SRC_FILES := ../AndroidLibs/$(TARGET_ARCH)/libandroid_runtime.17.so
endif
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= $(AOSP_PATH)/external/skia/include/core \
        $(AOSP_PATH)/frameworks/base/core/jni/android/graphics \
        $(AOSP_PATH)/frameworks/native/include \
        $(AOSP_PATH)/system/core/include		

LOCAL_SHARED_LIBRARIES := \
                libskia.$(API) \
      libandroid_runtime.$(API)
		
LOCAL_MODULE    := ndkbitmap.$(API)
LOCAL_SRC_FILES := NativeBitmapFactory.cpp



LOCAL_LDLIBS :=-llog #-lcutils -lskia -ljnigraphics -landroid_runtime
include $(BUILD_SHARED_LIBRARY)
