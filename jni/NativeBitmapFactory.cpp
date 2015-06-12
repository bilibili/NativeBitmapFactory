//#include <stdio.h>
//#include <stdlib.h>
#include <jni.h>
#include <dlfcn.h>
//#include <SkBitmap.h>
//#include <GraphicsJNI.h>
#include <stdint.h>
#include <malloc.h>
#include <android/log.h>
#include "NativeBitmapFactory.h"

#define SUCCESS 0
#define EGENERIC -1
#define ENOMEM -2
#define SIZE_OF_SKBITMAP 256

//api11: _ZN8SkBitmapC1Ev
//api14: //api15://api16://api17://api18://api19:
typedef void (*SkBitmap_ctor)(void *);

//api11: _ZN8SkBitmapD1Ev
//api14://api15://api16://api17://api18: //api19:
typedef void (*SkBitmap_dtor)(void *);

//api11: _ZN8SkBitmap9setConfigENS_6ConfigEiii
//api14://api15://api16://api17://api18:
//api19: _ZN8SkBitmap9setConfigENS_6ConfigEiij
typedef void (*SkBitmap_setConfig)(void *, int, int, int, int);

//api19-2(4.4.4): _ZN8SkBitmap9setConfigENS_6ConfigEiij11SkAlphaType
typedef void (*SkBitmap_setConfig_19later)(void *, int, int, int, size_t, uint8_t);

//API21: _ZN8SkBitmap7setInfoERK11SkImageInfoj
typedef void (*SkBitmap_setInfo)(void *, int *, int);
// typedef int (*SkImageInfo_make)(int, int, int, int);

//api11: _ZN8SkBitmap11allocPixelsEPNS_9AllocatorEP12SkColorTable
//api14: //api15://api16://api17://api18://api19
typedef void (*SkBitmap_allocPixels)(void *, void *, void *);

//api11: _ZNK8SkBitmap9eraseARGBEjjjj
//api14://api15://api16://api17://api18://api19:
typedef void (*SkBitmap_eraseARGB)(void *, size_t, size_t, size_t, size_t);

//API11: _ZN11GraphicsJNI12createBitmapEP7_JNIEnvP8SkBitmapbP11_jbyteArrayi
//API19: _ZN11GraphicsJNI12createBitmapEP7_JNIEnvP8SkBitmapiP11_jbyteArrayi 
typedef jobject (*GraphicsJNI_createBitmap)(void *, void *, int, void *, int);

//API19: _ZN11GraphicsJNI12createBitmapEP7_JNIEnvP8SkBitmapP11_jbyteArrayiS5_P10_jintArrayi
//API21: _ZN11GraphicsJNI12createBitmapEP7_JNIEnvP8SkBitmapP11_jbyteArrayiS5_P8_jobjecti
typedef jobject (*GraphicsJNI_createBitmap_19later)(void *, void *, void *, int, void *, void *, int);

typedef jobject (*GraphicsJNI_createBitmap_Mlater)(JNIEnv* env, void*, jbyteArray, int, jbyteArray, jobject, int, int, int);


/**
 *  Describes how to interpret the alpha compoent of a pixel.
 */
enum SkAlphaType
{
    /**
     *  All pixels should be treated as opaque, regardless of the value stored
     *  in their alpha field. Used for legacy images that wrote 0 or garbarge
     *  in their alpha field, but intended the RGB to be treated as opaque.
     */
    kIgnore_SkAlphaType,

    /**
     *  All pixels are stored as opaque. This differs slightly from kIgnore in
     *  that kOpaque has correct "opaque" values stored in the pixels, while
     *  kIgnore may not, but in both cases the caller should treat the pixels
     *  as opaque.
     */
    kOpaque_SkAlphaType,

    /**
     *  All pixels have their alpha premultiplied in their color components.
     *  This is the natural format for the rendering target pixels.
     */
    kPremul_SkAlphaType,

    /**
     *  All pixels have their color components stored without any regard to the
     *  alpha. e.g. this is the default configuration for PNG images.
     *
     *  This alpha-type is ONLY supported for input images. Rendering cannot
     *  generate this on output.
     */
    kUnpremul_SkAlphaType,

    kLastEnum_SkAlphaType = kUnpremul_SkAlphaType
};

/**
 *  Describes how to interpret the components of a pixel.
 */
enum SkColorType {
    kUnknown_SkColorType,
    kAlpha_8_SkColorType,
    kRGB_565_SkColorType,
    kARGB_4444_SkColorType,
    kRGBA_8888_SkColorType,
    kBGRA_8888_SkColorType,
    kIndex_8_SkColorType,

    kLastEnum_SkColorType = kIndex_8_SkColorType,

    kN32_SkColorType = kRGBA_8888_SkColorType,

#ifdef SK_SUPPORT_LEGACY_N32_NAME
    kPMColor_SkColorType = kN32_SkColorType
#endif
};

static int SkBitmapConfigToColorType(int config) {
    static const SkColorType gCT[] = {
        kUnknown_SkColorType,   // kNo_Config
        kAlpha_8_SkColorType,   // kA8_Config
        kIndex_8_SkColorType,   // kIndex8_Config
        kRGB_565_SkColorType,   // kRGB_565_Config
        kARGB_4444_SkColorType, // kARGB_4444_Config
        kN32_SkColorType,   // kARGB_8888_Config
    };
    return gCT[config];
};

struct skbitmap_sys_t
{
    void *libskia;

    SkBitmap_ctor sk_ctor;
    SkBitmap_dtor sk_dtor;
    SkBitmap_setConfig sk_setConfig;
    SkBitmap_setConfig_19later sk_setConfig_19later;
    SkBitmap_setInfo sk_setInfo;
    SkBitmap_allocPixels sk_allocPixels;
    SkBitmap_eraseARGB sk_eraseARGB;

    void *libjnigraphics;
    GraphicsJNI_createBitmap gjni_createBitmap;
    GraphicsJNI_createBitmap_19later gjni_createBitmap_19later;
    GraphicsJNI_createBitmap_Mlater gjni_createBitmap_Mlater;
};

struct ndkbitmap_object_t
{
    skbitmap_sys_t *sys;
};


enum BitmapCreateFlags {
    kBitmapCreateFlag_None = 0x0,
    kBitmapCreateFlag_Mutable = 0x1,
    kBitmapCreateFlag_Premultiplied = 0x2,
};

static jclass   gBitmap_class;
static jmethodID gBitmap_constructorMethodID;

static jclass make_globalref(JNIEnv* env, const char classname[])
{
    jclass c = env->FindClass(classname);
    if(c) {
        return (jclass)env->NewGlobalRef(c);
    }
    return NULL;
}

static jobject createBitmapM(JNIEnv* env, void* bitmap, jbyteArray buffer,
        int bitmapCreateFlags, jbyteArray ninePatchChunk, jobject ninePatchInsets, int density, int width, int height)
{
    bool isMutable = bitmapCreateFlags & kBitmapCreateFlag_Mutable;
    bool isPremultiplied = bitmapCreateFlags & kBitmapCreateFlag_Premultiplied;
    __android_log_print(ANDROID_LOG_INFO, "NativeBitmapFactory", "createBitmapM %d-%d", isMutable, isPremultiplied);
    jobject obj = env->NewObject(gBitmap_class, gBitmap_constructorMethodID,
            reinterpret_cast<jlong>(bitmap), buffer,
            width, height, density, isMutable, isPremultiplied,
            ninePatchChunk, ninePatchInsets);
    __android_log_print(ANDROID_LOG_INFO, "NativeBitmapFactory", "createBitmapM end");
    return obj;
}


static void *InitLibrary(struct skbitmap_sys_t *p_sys)
{
    /* DL Open libskia */
    void *p_library;
    p_library = dlopen("libskia.so", RTLD_NOW | RTLD_LOCAL);
    if (!p_library)
        return NULL;

    /* Register symbols */
    p_sys->sk_ctor = (SkBitmap_ctor)(dlsym(p_library, "_ZN8SkBitmapC1Ev"));
    p_sys->sk_dtor = (SkBitmap_dtor)(dlsym(p_library, "_ZN8SkBitmapD1Ev"));
    p_sys->sk_setConfig = (SkBitmap_setConfig)(dlsym(p_library, "_ZN8SkBitmap9setConfigENS_6ConfigEiii"));
    if (!p_sys->sk_setConfig)
    {
        p_sys->sk_setConfig = (SkBitmap_setConfig)(dlsym(p_library, "_ZN8SkBitmap9setConfigENS_6ConfigEiij"));
    }
    p_sys->sk_setConfig_19later = (SkBitmap_setConfig_19later)(dlsym(p_library, "_ZN8SkBitmap9setConfigENS_6ConfigEiij11SkAlphaType"));
    if (!p_sys->sk_setConfig && !p_sys->sk_setConfig_19later)
    {
        p_sys->sk_setInfo = (SkBitmap_setInfo)(dlsym(p_library, "_ZN8SkBitmap7setInfoERK11SkImageInfoj"));
    }

    p_sys->sk_allocPixels = (SkBitmap_allocPixels)(dlsym(p_library, "_ZN8SkBitmap11allocPixelsEPNS_9AllocatorEP12SkColorTable"));
    if (!p_sys->sk_allocPixels) {    // for Nexus Android M Preview
        p_sys->sk_allocPixels = (SkBitmap_allocPixels)(dlsym(p_library, "_ZN8SkBitmap14tryAllocPixelsEPNS_9AllocatorEP12SkColorTable"));
    }
    p_sys->sk_eraseARGB = (SkBitmap_eraseARGB)(dlsym(p_library, "_ZNK8SkBitmap9eraseARGBEjjjj"));

    __android_log_print(ANDROID_LOG_INFO, "NativeBitmapFactory", "[libskia] ctor:%p,dtor:%p,setConfig:%p,setConfig_19later:%p,setInfo:%p,allocPixels:%p,eraseARGB:%p",
                        p_sys->sk_ctor, p_sys->sk_dtor,
                        p_sys->sk_setConfig, p_sys->sk_setConfig_19later, p_sys->sk_setInfo,
                        p_sys->sk_allocPixels, p_sys->sk_eraseARGB);
    // We need all the Symbols
    if (!(p_sys->sk_ctor && p_sys->sk_dtor
            && (p_sys->sk_setConfig || p_sys->sk_setConfig_19later || p_sys->sk_setInfo)
            && p_sys->sk_allocPixels && p_sys->sk_eraseARGB))
    {
        __android_log_print(ANDROID_LOG_ERROR, "NativeBitmapFactory", "InitLibrary dlsym failed");
        dlclose(p_library);
        return NULL;
    }
    return p_library;
}

static void *InitLibrary2(struct skbitmap_sys_t *p_sys, JNIEnv *env)
{
    /* DL Open libjnigraphics */
    void *p_library;
    p_library = dlopen("libandroid_runtime.so", RTLD_NOW | RTLD_LOCAL);
    if (!p_library)
        return NULL;

    /* Register symbols */
    p_sys->gjni_createBitmap = (GraphicsJNI_createBitmap)(dlsym(p_library, "_ZN11GraphicsJNI12createBitmapEP7_JNIEnvP8SkBitmapbP11_jbyteArrayi"));
    if(!p_sys->gjni_createBitmap) 
    {
        p_sys->gjni_createBitmap = (GraphicsJNI_createBitmap)(dlsym(p_library, "_ZN11GraphicsJNI12createBitmapEP7_JNIEnvP8SkBitmapiP11_jbyteArrayi"));
    }

    p_sys->gjni_createBitmap_19later = (GraphicsJNI_createBitmap_19later)(dlsym(p_library, "_ZN11GraphicsJNI12createBitmapEP7_JNIEnvP8SkBitmapP11_jbyteArrayiS5_P10_jintArrayi"));
    if(!p_sys->gjni_createBitmap_19later) 
    {
        p_sys->gjni_createBitmap_19later = (GraphicsJNI_createBitmap_19later)(dlsym(p_library, "_ZN11GraphicsJNI12createBitmapEP7_JNIEnvP8SkBitmapP11_jbyteArrayiS5_P8_jobjecti"));
    }

    if (!p_sys->gjni_createBitmap && !p_sys->gjni_createBitmap_19later) {
        if (!gBitmap_class) {
            gBitmap_class = make_globalref(env, "android/graphics/Bitmap");
        }
        if (gBitmap_class) {
            gBitmap_constructorMethodID = env->GetMethodID(gBitmap_class, "<init>", "(J[BIIIZZ[BLandroid/graphics/NinePatch$InsetStruct;)V");
            jthrowable jexception = env->ExceptionOccurred();
            if (jexception) {
                 // __android_log_print(ANDROID_LOG_INFO, "NativeBitmapFactory", "gBitmap_class:%p, gBitmap_constructorMethodID:%p", gBitmap_class, gBitmap_constructorMethodID);
                env->ExceptionDescribe();
                env->ExceptionClear();
            } else {
                p_sys->gjni_createBitmap_Mlater = createBitmapM;
            }
        }
    }

    __android_log_print(ANDROID_LOG_INFO, "NativeBitmapFactory", "[GraphicsJNI] createBitmap:%p,createBitmap_19later:%p,createBitmap_Mlater:%p",
                        p_sys->gjni_createBitmap,
                        p_sys->gjni_createBitmap_19later,
                        p_sys->gjni_createBitmap_Mlater);

    // We need all the Symbols
    if (!p_sys->gjni_createBitmap && !p_sys->gjni_createBitmap_19later && !p_sys->gjni_createBitmap_Mlater)
    {
        __android_log_print(ANDROID_LOG_ERROR, "NativeBitmapFactory", "InitLibrary2 dlsym failed");
        p_sys->gjni_createBitmap = NULL;
        p_sys->gjni_createBitmap_19later = NULL;
        p_sys->gjni_createBitmap_Mlater = NULL;
        dlclose(p_library);
        return NULL;
    }
    return p_library;
}

ndkbitmap_object_t *ndkbitmap_obj;


static int Open(ndkbitmap_object_t *obj, JNIEnv *env)
{
    skbitmap_sys_t *sys = (skbitmap_sys_t *)malloc(sizeof (*sys));

    if (sys == NULL)
        return ENOMEM;

    sys->libskia = InitLibrary(sys);
    if (sys->libskia == NULL)
    {
        free(sys);
        return EGENERIC;
    }
    sys->libjnigraphics = InitLibrary2(sys, env);
    if (sys->libjnigraphics == NULL)
    {
        free(sys);
        return EGENERIC;
    }

    obj->sys = sys;
    return SUCCESS;
}

static void Close(ndkbitmap_object_t *obj)
{
    skbitmap_sys_t *sys = obj->sys;

    if (sys == NULL) {
        return;
    }
    if (sys->libjnigraphics) {
        dlclose(sys->libjnigraphics);
        sys->libjnigraphics = NULL;
    }
    if (sys->libskia) {
        dlclose(sys->libskia);
        sys->libskia = NULL;
    }
    free(sys);
}

static int Start(JNIEnv *env)
{
    ndkbitmap_obj = (ndkbitmap_object_t *)malloc(sizeof(*ndkbitmap_obj));
    int r = Open(ndkbitmap_obj, env);
    if (r != SUCCESS)
    {
        if (ndkbitmap_obj){
            free(ndkbitmap_obj);
            ndkbitmap_obj = NULL;
        }
        return ENOMEM;
    }
    return SUCCESS;
}

static int Stop()
{
    if (ndkbitmap_obj == NULL)
    {
        return SUCCESS;
    }
    Close(ndkbitmap_obj);
    free(ndkbitmap_obj);
    ndkbitmap_obj = NULL;
    return SUCCESS;
}


// inline void *createBitmap2(int config, int w, int h)
// {
//     SkBitmap *bm = new SkBitmap();
//     bm->setConfig(config, w, h);
//     //bm->setIsOpaque(!hasAlpha);
//     bm->allocPixels();
//     //bm->eraseColor(0);
//     bm->eraseARGB(0, 0, 0, 0);
// }

inline void *createSkBitmap(ndkbitmap_object_t *obj, int config, int w, int h)
{
    skbitmap_sys_t *p_sys = obj->sys;
    if (p_sys == NULL || p_sys->libskia == NULL)
    {
        return NULL;
    }
    void *skBitmap = malloc(SIZE_OF_SKBITMAP);
    if (!skBitmap)
    {
        return NULL;
    }
    *((uint32_t *) ((uint32_t)skBitmap + SIZE_OF_SKBITMAP - 4)) = 0xbaadbaad;
    //ctor
    p_sys->sk_ctor(skBitmap);
    if (p_sys->sk_setConfig)
    {
        p_sys->sk_setConfig(skBitmap, config, w, h, 0);
    }
    else if (p_sys->sk_setConfig_19later)
    {
        p_sys->sk_setConfig_19later(skBitmap, config, w, h, 0, (uint8_t)kPremul_SkAlphaType);
    } else if (p_sys->sk_setInfo) 
    {
        int imageInfo[4] = {w, h, SkBitmapConfigToColorType(config), kPremul_SkAlphaType};
        p_sys->sk_setInfo(skBitmap, imageInfo, 0);
    }
    p_sys->sk_allocPixels(skBitmap, NULL, NULL);
    p_sys->sk_eraseARGB(skBitmap, 0, 0, 0, 0);


    if (!(*((uint32_t *) ((uint32_t)skBitmap + SIZE_OF_SKBITMAP - 4)) == 0xbaadbaad) )
    {
        free(skBitmap);
        return NULL;
    }

    return skBitmap;
}

jboolean Java_tv_cjump_jni_NativeBitmapFactory_init(JNIEnv *env)
{
#if defined(__arm__)
    __android_log_print(ANDROID_LOG_INFO, "NativeBitmapFactory", "Loaded libndkbitmap.so arch is: ARM");
#elif defined(__i386__)
    __android_log_print(ANDROID_LOG_INFO, "NativeBitmapFactory", "Loaded libndkbitmap.so arch is: x86");
#endif
    int r = Start(env);
    return r == SUCCESS;
}

jboolean Java_tv_cjump_jni_NativeBitmapFactory_release(JNIEnv *env)
{
    int r = Stop();
    return r == SUCCESS;
}

jobject createBitmap(JNIEnv *env , jobject  obj, jint w, jint h, jint config, jboolean hasAlpha, int isMuttable, int api)
{
    void *bm = createSkBitmap(ndkbitmap_obj, config, w, h);
    if (bm == NULL)
    {
        return NULL;
    }
    jobject result = NULL;
    skbitmap_sys_t *p_sys = ndkbitmap_obj->sys;
    if(p_sys->libjnigraphics) 
    {
        if(p_sys->gjni_createBitmap) 
        {
            result = p_sys->gjni_createBitmap(env, bm, isMuttable, NULL, -1);
        } else if(p_sys->gjni_createBitmap_19later) {
            result = p_sys->gjni_createBitmap_19later(env, bm, NULL, isMuttable, NULL, NULL, -1);
        } else if(p_sys->gjni_createBitmap_Mlater) {
            result = p_sys->gjni_createBitmap_Mlater(env, bm, NULL, isMuttable, NULL, NULL, -1, w, h);
        }

    }
    
    return result;
}

jobject Java_tv_cjump_jni_NativeBitmapFactory_createBitmap(JNIEnv *env , jobject  obj, jint w, jint h, jint config, jboolean hasAlpha)
{
    return createBitmap(env, obj, w, h, config, hasAlpha, true, 0);
}

jobject Java_tv_cjump_jni_NativeBitmapFactory_createBitmap19(JNIEnv *env , jobject  obj, jint w, jint h, jint config, jboolean hasAlpha)
{
    return createBitmap(env, obj, w, h, config, hasAlpha, 0x3, 19);
}