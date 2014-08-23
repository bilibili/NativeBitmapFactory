LOCAL_PATH :=$(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= $(AOSP_PATH)/frameworks/base/core/jni/android/graphics \
					$(AOSP_PATH)/external/skia/include/core \
					$(AOSP_PATH)/external/skia/include/images \
					$(AOSP_PATH)/external/skia/include/lazy \
					$(AOSP_PATH)/frameworks/base/include \
					$(AOSP_PATH)/system/core/include \
		
LOCAL_MODULE    := ndkbitmap
LOCAL_SRC_FILES := NativeBitmapFactory.cpp



LOCAL_LDLIBS := -L$(AOSP_LIBS_PATH)/armeabi -llog -lskia -landroid_runtime
include $(BUILD_SHARED_LIBRARY)
