LOCAL_PATH :=$(call my-dir)

include $(CLEAR_VARS)
ifneq ($(ARCH_ARM_HAVE_VFP),true)
	LOCAL_CFLAGS += -DSK_SOFTWARE_FLOAT
endif

ifeq ($(ARCH_ARM_HAVE_NEON),true)
	LOCAL_CFLAGS += -D__ARM_HAVE_NEON
endif

LOCAL_C_INCLUDES += $(AOSP_PATH)/external/skia/include/core \
					$(AOSP_PATH)/frameworks/base/native/include \
					$(AOSP_PATH)/frameworks/base/core/jni/android/graphics \
					$(AOSP_PATH)/frameworks/native/include \
					$(AOSP_PATH)/external/skia/include/images \
					$(AOSP_PATH)/external/skia/include/lazy \
					$(AOSP_PATH)/frameworks/base/include \
					$(AOSP_PATH)/system/core/include \
		
LOCAL_MODULE    := ndkbitmap
LOCAL_SRC_FILES := NativeBitmapFactory.cpp



LOCAL_LDLIBS := -L$(AOSP_LIBS_PATH)/armeabi -llog #-lskia.$(API) -landroid_runtime.$(API)
include $(BUILD_SHARED_LIBRARY)
