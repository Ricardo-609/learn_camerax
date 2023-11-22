#include <jni.h>
#include "drawer/triangle_drawer.h"
#include "utils/logger.h"
#include "render/image_render.h"
// 需要加上这个宏不然编译器会编译失败
#define STB_IMAGE_IMPLEMENTATION

#include "utils/stb_image.h"
#include "filter/adjust/color_invert_filter.h"
#include "filter/adjust/contrast_image_filter.h"
#include "filter/adjust/exposure_filter.h"
#include "filter/adjust/brightness_filter.h"
#include "filter/adjust/saturation_filter.h"
#include "filter/adjust/hue_filter.h"
#include "filter/adjust/sharpen_filter.h"
#include "egl/gl_render.h"
#include <android/bitmap.h>
#include <malloc.h>
#include <string.h>

extern "C"
JNIEXPORT jint JNICALL
Java_com_ricardo_learn_1camerax_MainActivity_createGLRender(JNIEnv *env, jobject thiz,
                                                             jobject surface) {
    // 创建渲染线程
    GLRender *glRender = new GLRender(env);

    glRender->SetSurface(surface);
    return (jlong) glRender;

}

extern "C"
JNIEXPORT void JNICALL
Java_com_ricardo_learn_1camerax_MainActivity_showBitmap(JNIEnv *env, jobject thiz, jint render,
                                                         jobject bitmap) {
    AndroidBitmapInfo info; // create a AndroidBitmapInfo
    int result;
    // 获取图片信息
    result = AndroidBitmap_getInfo(env, bitmap, &info);
    if (result != ANDROID_BITMAP_RESULT_SUCCESS) {
        LOGE("Player", "AndroidBitmap_getInfo failed, result: %d", result);
        return;
    }
    LOGD("Player", "bitmap width: %d, height: %d, format: %d, stride: %d", info.width, info.height,
         info.format, info.stride);
    // 获取像素信息
    unsigned char *data;

    result = AndroidBitmap_lockPixels(env, bitmap, reinterpret_cast<void **>(&data));
    if (result != ANDROID_BITMAP_RESULT_SUCCESS) {
        LOGE("Player", "AndroidBitmap_lockPixels failed, result: %d", result);
        return;
    }
    size_t count = info.stride * info.height;

    unsigned char *resultData = (unsigned char *) malloc(count * sizeof(unsigned char));

    memcpy(resultData, data, count);

    // 像素信息不再使用后需要解除锁定
    result = AndroidBitmap_unlockPixels(env, bitmap);
    if (result != ANDROID_BITMAP_RESULT_SUCCESS) {
        LOGE("Player", "AndroidBitmap_unlockPixels failed, result: %d", result);
    }

    ImageRender *pImageRender = new ImageRender(info.width, info.height, resultData);
    GLRender *pGLRender = reinterpret_cast<GLRender *>(render);
    pGLRender->SetBitmapRender(pImageRender);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ricardo_learn_1camerax_MainActivity_releaseGLRender(JNIEnv *env, jobject thiz,
                                                              jint render) {
    GLRender *pGLRender = reinterpret_cast<GLRender *>(render);
    pGLRender->ReleaseRender();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ricardo_learn_1camerax_MainActivity_switchToFilterNative(JNIEnv *env, jobject thiz,
                                                                   jint render, jint filter_type) {

    GLRender *pGLRender = reinterpret_cast<GLRender *>(render);
    ImageRender *pRender = pGLRender->GetImageRender();
    if (pRender != NULL) {
        ImageFilter *pImageFilter = NULL;

        switch (filter_type) {
            case 0:
                pImageFilter = new ImageFilter();
                break;
            case 1:
                pImageFilter = new ColorInvertImageFilter();
                break;
            case 2:
                pImageFilter = new ContrastImageFilter();
                break;
            case 3:
                pImageFilter = new BrightnessFilter();
                break;
            case 4:
                pImageFilter = new ExposureFilter();
                break;
            case 5:
                pImageFilter = new HueFilter();
                break;
            case 6:
                pImageFilter = new SaturationFilter();
                break;
            case 7:
                pImageFilter = new SharpenFilter();
                break;
        }
        pRender->setFilter(pImageFilter);


    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_ricardo_learn_1camerax_MainActivity_adjust(JNIEnv *env, jobject thiz, jint render,
                                                     jfloat value, jint filter_type) {
    GLRender *pGLRender = reinterpret_cast<GLRender *>(render);
    ImageRender *pRender = pGLRender->GetImageRender();
    if (pRender != NULL) {
        jlong pFilter = reinterpret_cast<jlong>(pRender->getFilter());

        switch (filter_type) {
            case 2: {
                auto *pImageFilter = reinterpret_cast<ContrastImageFilter *>(pFilter);
                if (pImageFilter != NULL)
                    pImageFilter->setValue(value);
            }
                break;
            case 3: {
                auto *pImageFilter = reinterpret_cast<BrightnessFilter *>(pFilter);
                if (pImageFilter != NULL)
                    pImageFilter->setValue(value);
            }
                break;
            case 4: {
                auto *pImageFilter = reinterpret_cast<ExposureFilter *>(pFilter);
                if (pImageFilter != NULL)
                    pImageFilter->setValue(value);
            }
                break;
            case 5: {
                auto *pImageFilter = reinterpret_cast<HueFilter *>(pFilter);
                if (pImageFilter != NULL)
                    pImageFilter->setValue(value);
            }
                break;
            case 6: {
                auto *pImageFilter = reinterpret_cast<SaturationFilter *>(pFilter);
                if (pImageFilter != NULL)
                    pImageFilter->setValue(value);
            }
                break;
            case 7: {
                auto *pImageFilter = reinterpret_cast<SharpenFilter *>(pFilter);
                if (pImageFilter != NULL)
                    pImageFilter->setValue(value);
            }
                break;
        }
    }

}