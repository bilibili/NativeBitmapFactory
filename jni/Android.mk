LOCAL_PATH :=$(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := libskia.$(API)
LOCAL_SRC_FILES := ../AndroidLibs/libskia.$(API).so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libandroid_runtime.$(API)
LOCAL_SRC_FILES := ../AndroidLibs/libandroid_runtime.$(API).so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

MY_LOCAL_ANDSRC :=/cygdrive/d/projects/ndk/NativeBitmapFactory/AndroidSource/platform
include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= \
        $(MY_LOCAL_ANDSRC)/external/skia/include/core \
        $(MY_LOCAL_ANDSRC)/frameworks/base/core/jni/android/graphics \
        $(MY_LOCAL_ANDSRC)/frameworks/native/include \
        $(MY_LOCAL_ANDSRC)/system/core/include		

LOCAL_SHARED_LIBRARIES := \
                libskia.$(API) \
      libandroid_runtime.$(API)
		
LOCAL_MODULE    := ndkbitmap.$(API)
LOCAL_SRC_FILES := NativeBitmapFactory.cpp



LOCAL_LDLIBS :=-llog #-lcutils -lskia -ljnigraphics -landroid_runtime
include $(BUILD_SHARED_LIBRARY)