#include "egl_core.h"
#include "../utils/logger.h"

EGLConfig EglCore::GetEGLConfig() {
    EGLint numConfigs;
    EGLConfig config;
    // 希望的最小配置
    static const EGLint CONFIG_ATTRIBS[] = {
            EGL_BUFFER_SIZE, EGL_DONT_CARE,
            EGL_RED_SIZE, 8,    // R 位数
            EGL_GREEN_SIZE, 8,  // G 位数
            EGL_BLUE_SIZE, 8,   // B 位数
            EGL_ALPHA_SIZE, 8,  // A 位数
            EGL_DEPTH_SIZE, 16, // 深度
            EGL_STENCIL_SIZE, EGL_DONT_CARE,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE // the end 结束标志
    };
    // 根据所设定的最小配置系统会选择一个满足你最低要求的配置，这个真正的配置往往要比你期望的属性更多
    EGLBoolean success = eglChooseConfig(m_egl_dsp, CONFIG_ATTRIBS, &config, 1, &numConfigs);
    if (!success || eglGetError() != EGL_SUCCESS) {
        LOGE(TAG, "EGL config fail")
        return NULL;
    }
    return config;
}

EglCore::EglCore() {

}

EglCore::~EglCore() {

}

bool EglCore::Init(EGLContext share_ctx) {
    if (m_egl_dsp != EGL_NO_DISPLAY) {
        LOGE(TAG, "EGL already set up")
        return true;
    }
    if (share_ctx == NULL) {
        share_ctx = EGL_NO_CONTEXT;
    }
    // 获取Display
    m_egl_dsp = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    if (m_egl_dsp == EGL_NO_DISPLAY || eglGetError() != EGL_SUCCESS) {
        LOGE(TAG, "EGL init display fail")
        return false;
    }

    EGLint major_ver, minor_ver;
    // 初始化egl
    EGLBoolean success = eglInitialize(m_egl_dsp, &major_ver, &minor_ver);
    if (success != EGL_TRUE || eglGetError() != EGL_SUCCESS) {
        LOGE(TAG, "EGL init fail")
        return false;
    }

    LOGI(TAG, "EGL version: %d.%d", major_ver, minor_ver)
    // 获取eglconfig
    m_egl_config = GetEGLConfig();

    const EGLint attr[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    // 创建EglContext
    m_egl_context = eglCreateContext(m_egl_dsp, m_egl_config, share_ctx, attr);
    if (m_egl_context == EGL_NO_CONTEXT) {
        LOGE(TAG, "EGL create fail, error is %x", eglGetError());
        return false;
    }

    EGLint egl_format;
    success = eglGetConfigAttrib(m_egl_dsp, m_egl_config, EGL_NATIVE_VISUAL_ID, &egl_format);
    if (success != EGL_TRUE || eglGetError() != EGL_SUCCESS) {
        LOGE(TAG, "EGL get config fail, error is %x", eglGetError())
        return false;
    }

    LOGI(TAG, "EGL init success")
    return true;
}

EGLSurface EglCore::CreateWindSurface(ANativeWindow *window) {
    // 调用EGL native API创建window surface
    EGLSurface surface = eglCreateWindowSurface(m_egl_dsp, m_egl_config, window, 0);
    if (eglGetError() != EGL_SUCCESS) {
        LOGI(TAG, "EGL create window surface fail")
        return NULL;
    }
    return surface;
}

EGLSurface EglCore::CreateOffScreenSurface(int width, int height) {
    int CONFIG_ATTRIBS[] = {
            EGL_WIDTH, width,
            EGL_HEIGHT, height,
            EGL_NONE
    };
    EGLSurface surface = eglCreatePbufferSurface(m_egl_dsp, m_egl_config, CONFIG_ATTRIBS);
    if (eglGetError() != EGL_SUCCESS) {
        LOGI(TAG, "EGL create window surface fail")
        return NULL;
    }
    return surface;
}

void EglCore::MakeCurrent(EGLSurface egl_surface) {
    // 调用EGL native API绑定渲染环境到当前线程
    if (!eglMakeCurrent(m_egl_dsp, egl_surface, egl_surface, m_egl_context)) {
        LOGE(TAG, "EGL make current fail");
    }
}

void EglCore::SwapBuffer(EGLSurface egl_surface) {
    eglSwapBuffers(m_egl_dsp, egl_surface);
}

void EglCore::DestroySurface(EGLSurface egl_surface) {
    eglMakeCurrent(m_egl_dsp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(m_egl_dsp, egl_surface);
}

void EglCore::Release() {
    if (m_egl_dsp != EGL_NO_DISPLAY) {
        eglMakeCurrent(m_egl_dsp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(m_egl_dsp, m_egl_context);
        eglReleaseThread();
        eglTerminate(m_egl_dsp);
    }
    m_egl_dsp = EGL_NO_DISPLAY;
    m_egl_context = EGL_NO_CONTEXT;
    m_egl_config = NULL;
}
