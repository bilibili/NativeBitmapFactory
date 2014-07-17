#include <stdio.h>
#include <stdlib.h>
#include <jni.h>
#include <SkBitmap.h>
#include <GraphicsJNI.h>
//#include <android/log.h>
#include "NativeBitmapFactory.h"

jobject createBitmap(JNIEnv * env ,jobject  obj,jint w,jint h,SkBitmap::Config config,jboolean hasAlpha,int isMuttable)
{
SkBitmap* bm = new SkBitmap();
bm->setConfig(config, w, h);
//bm->setIsOpaque(!hasAlpha);
bm->allocPixels();
bm->eraseColor(0);
//__android_log_print(ANDROID_LOG_DEBUG, "NativeBitmap", "Created bitmap %d has width = %d, height = %d",bm, bm->width(), bm->height());
jobject result = GraphicsJNI::createBitmap(env,bm,isMuttable,NULL,-1);
return result;
}

jobject Java_tv_cjump_jni_NativeBitmapFactory_createBitmap(JNIEnv * env ,jobject  obj,jint w,jint h,SkBitmap::Config config,jboolean hasAlpha)
{
return createBitmap(env,obj,w,h,config,hasAlpha,true);
}

jobject Java_tv_cjump_jni_NativeBitmapFactory_createBitmap19(JNIEnv * env ,jobject  obj,jint w,jint h,SkBitmap::Config config,jboolean hasAlpha)
{
return createBitmap(env,obj,w,h,config,hasAlpha,0x3);
}