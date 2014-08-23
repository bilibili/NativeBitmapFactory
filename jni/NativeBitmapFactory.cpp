//#include <stdio.h>
//#include <stdlib.h>
#include <jni.h>
#include <dlfcn.h>
//#include <SkBitmap.h>
#include <GraphicsJNI.h>
//#include <android/log.h>
#include "NativeBitmapFactory.h"

#define SUCCESS 0
#define EGENERIC -1
#define ENOMEM -2
#define SIZE_OF_SKBITMAP 1024

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

//api11: _ZN8SkBitmap11allocPixelsEPNS_9AllocatorEP12SkColorTable
//api14: //api15://api16://api17://api18://api19
typedef void (*SkBitmap_allocPixels)(void *, void *, void *);

//api11: _ZNK8SkBitmap9eraseARGBEjjjj
//api14://api15://api16://api17://api18://api19:
typedef void (*SkBitmap_eraseARGB)(void *, size_t, size_t, size_t, size_t);

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

struct skbitmap_sys_t
{
    void *libskia;

    SkBitmap_ctor sk_ctor;
    SkBitmap_dtor sk_dtor;
    SkBitmap_setConfig sk_setConfig;
    SkBitmap_setConfig_19later sk_setConfig_19later;
    SkBitmap_allocPixels sk_allocPixels;
    SkBitmap_eraseARGB sk_eraseARGB;
};

struct ndkbitmap_object_t
{
    skbitmap_sys_t *sys;
};

static void *InitLibrary(struct skbitmap_sys_t *p_sys)
{
    /* DL Open libmedia */
    void *p_library;
    p_library = dlopen("libskia.so", RTLD_NOW | RTLD_LOCAL);
    if (!p_library)
        return NULL;

    /* Register symbols */
    p_sys->sk_ctor = (SkBitmap_ctor)(dlsym(p_library, "_ZN8SkBitmapC1Ev"));
    p_sys->sk_dtor = (SkBitmap_dtor)(dlsym(p_library, "_ZN8SkBitmapD1Ev"));
    p_sys->sk_setConfig = (SkBitmap_setConfig)(dlsym(p_library, "_ZN8SkBitmap9setConfigENS_6ConfigEiii"));
    if (p_sys->sk_setConfig == NULL)
    {
        p_sys->sk_setConfig = (SkBitmap_setConfig)(dlsym(p_library, "_ZN8SkBitmap9setConfigENS_6ConfigEiij"));
    }
    p_sys->sk_setConfig_19later = (SkBitmap_setConfig_19later)(dlsym(p_library, "_ZN8SkBitmap9setConfigENS_6ConfigEiij11SkAlphaType"));
    p_sys->sk_allocPixels = (SkBitmap_allocPixels)(dlsym(p_library, "_ZN8SkBitmap11allocPixelsEPNS_9AllocatorEP12SkColorTable"));
    p_sys->sk_eraseARGB = (SkBitmap_eraseARGB)(dlsym(p_library, "_ZNK8SkBitmap9eraseARGBEjjjj"));

    // We need all the Symbols
    if (!(p_sys->sk_ctor && p_sys->sk_dtor
            && (p_sys->sk_setConfig || p_sys->sk_setConfig_19later)
            && p_sys->sk_allocPixels && p_sys->sk_eraseARGB))
    {
        dlclose(p_library);
        return NULL;
    }
    return p_library;
}

ndkbitmap_object_t *ndkbitmap_obj;


static int Open(ndkbitmap_object_t *obj)
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

    obj->sys = sys;
    return SUCCESS;
}

static void Close(ndkbitmap_object_t *obj)
{
    skbitmap_sys_t *sys = obj->sys;

    dlclose(sys->libskia);
    free(sys);
}

static int Start()
{
    ndkbitmap_obj = (ndkbitmap_object_t *)malloc(sizeof(*ndkbitmap_obj));
    int r = Open(ndkbitmap_obj);
    if (r != SUCCESS)
    {
        if (ndkbitmap_obj)
            free(ndkbitmap_obj);
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
    int r = Start();
    return r == SUCCESS;
}

jboolean Java_tv_cjump_jni_NativeBitmapFactory_release(JNIEnv *env)
{
    int r = Stop();
    return r == SUCCESS;
}

jobject createBitmap(JNIEnv *env , jobject  obj, jint w, jint h, jint config, jboolean hasAlpha, int isMuttable)
{
    void *bm = createSkBitmap(ndkbitmap_obj, config, w, h);
    //__android_log_print(ANDROID_LOG_DEBUG, "NativeBitmap", "Created bitmap %d has width = %d, height = %d",bm, bm->width(), bm->height());
    if (bm == NULL)
    {
        return NULL;
    }
    jobject result = GraphicsJNI::createBitmap(env, (SkBitmap*)bm, isMuttable, NULL, -1);
    return result;
}

jobject Java_tv_cjump_jni_NativeBitmapFactory_createBitmap(JNIEnv *env , jobject  obj, jint w, jint h, jint config, jboolean hasAlpha)
{
    return createBitmap(env, obj, w, h, config, hasAlpha, true);
}

jobject Java_tv_cjump_jni_NativeBitmapFactory_createBitmap19(JNIEnv *env , jobject  obj, jint w, jint h, jint config, jboolean hasAlpha)
{
    return createBitmap(env, obj, w, h, config, hasAlpha, 0x3);
}