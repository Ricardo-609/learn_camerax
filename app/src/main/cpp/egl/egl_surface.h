#include <android/native_window.h>
#include "egl_core.h"

#ifndef WHEAT_EGL_SURFACE_H
#define WHEAT_EGL_SURFACE_H


class EglSurface {
private:
    const char *TAG = "EglSurface";
    // 本地屏幕
    ANativeWindow *m_native_window = NULL;
    // 封装了EGLDisplay EGLConfig EGLContext的自定义类
    EglCore *m_core;
    // 渲染缓存，一块内存空间，所有要渲染到屏幕上的图像数据，都要先缓存在EGLSurface上渲染缓存，一块内存空间，
    // 所有要渲染到屏幕上的图像数据，都要先缓存在EGLSurface上
    // EGL API通过的EGLSurface
    EGLSurface m_surface;
public:
    EglSurface();
    ~EglSurface();

    bool Init();
    void CreateEglSurface(ANativeWindow *native_window, int width, int height);
    void MakeCurrent();
    void SwapBuffers();
    void DestroyEglSurface();
    void Release();
};


#endif //WHEAT_EGL_SURFACE_H
