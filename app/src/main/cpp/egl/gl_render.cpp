#include <unistd.h>
#include <GLES2/gl2.h>
#include "gl_render.h"
#include "../utils/logger.h"
#include "../render/image_render.h"
#include "../drawer/triangle_drawer.h"

void GLRender::InitRenderThread() {
    // 使用智能指针，线程结束时，自动删除本类指针
    std::shared_ptr<GLRender> that(this);
    std::thread t(sRenderThread, that);
    t.detach();
}

GLRender::GLRender(JNIEnv *env) {
    this->m_env = env;
    //获取JVM虚拟机，为创建线程作准备
    env->GetJavaVM(&m_jvm_for_thread);
    InitRenderThread();
}

GLRender::~GLRender() {
    delete m_egl_surface;
}


void GLRender::sRenderThread(std::shared_ptr<GLRender> that) {
    JNIEnv *env;
    // （1）将线程附加到虚拟机，并获取env
    if (that->m_jvm_for_thread->AttachCurrentThread(&env, NULL) != JNI_OK) {
        LOGE(that->TAG, "线程初始化异常");
        return;
    }

    // （2）初始化 EGL
    if (!that->InitEGL()) {
        //解除线程和jvm关联
        that->m_jvm_for_thread->DetachCurrentThread();
        return;
    }
    // 进入循环
    while (true) {
        // 根据opengl渲染状态进入不同的处理
        switch (that->m_state) {
            // 刷新surface,从外面surface后m_state置为该状态，说明已从外部（java层）获得surface对象
            case FRESH_SURFACE:
                LOGI(that->TAG, "Loop Render FRESH_SURFACE")
                that->InitDspWindow(env);       // （3）初始化window
                that->CreateSurface();          // （4）创建EglSurface
                that->m_state = RENDERING;      // m_state置为RENDRING状态进入渲染
                break;
            case RENDERING:
                LOGI(that->TAG, "Loop Render RENDERING")
                // （5）渲染
                that->Render();
                break;
            case SURFACE_DESTROY:
                LOGI(that->TAG, "Loop Render SURFACE_DESTROY")
                // （7）释放资源
                that->DestroySurface();
                that->m_state = NO_SURFACE;
                break;
            case STOP:
                LOGI(that->TAG, "Loop Render STOP")
                // （6）解除线程和jvm关联
                that->ReleaseRender();
                that->m_jvm_for_thread->DetachCurrentThread();
                return;
            case NO_SURFACE:
            default:
                break;
        }
        usleep(20000);
    }
}
// 初始化EGL
bool GLRender::InitEGL() {
    // 创建EglSurface对象
    m_egl_surface = new EglSurface();
    // 调用EglSurface的init方法
    return m_egl_surface->Init();
}

void GLRender::SetSurface(jobject surface) {
    if (NULL != surface) {
        m_surface_ref = m_env->NewGlobalRef(surface);
        m_state = FRESH_SURFACE;
    } else {
        m_env->DeleteGlobalRef(m_surface_ref);
        m_state = SURFACE_DESTROY;
    }
}
// 创建Window
void GLRender::InitDspWindow(JNIEnv *env) {
    // 传进来surface对象的引用
    if (m_surface_ref != NULL) {
        // 初始化窗口
        m_native_window = ANativeWindow_fromSurface(env, m_surface_ref);

        // 绘制区域的宽高
        m_window_width = ANativeWindow_getWidth(m_native_window);
        m_window_height = ANativeWindow_getHeight(m_native_window);

        //设置宽高限制缓冲区中的像素数量
        ANativeWindow_setBuffersGeometry(m_native_window, m_window_width,
                                         m_window_height, WINDOW_FORMAT_RGBA_8888);

        LOGD(TAG, "View Port width: %d, height: %d", m_window_width, m_window_height)
    }
}
// 创建EglSurface并绑定到线程
void GLRender::CreateSurface() {
    m_egl_surface->CreateEglSurface(m_native_window, m_window_width, m_window_height);
    glViewport(0, 0, m_window_width, m_window_height);
}


void GLRender::DestroySurface() {
    m_egl_surface->DestroyEglSurface();
    ReleaseWindow();
}


void GLRender::Render() {
    if (RENDERING == m_state) {
        if (pImageRender != NULL) {
            pImageRender->DoDraw();
        }
        m_egl_surface->SwapBuffers();
    }
}

void GLRender::Stop() {
    m_state = STOP;
}

void GLRender::ReleaseRender() {
    Stop();
    ReleaseSurface();
    ReleaseWindow();
}

void GLRender::ReleaseSurface() {
    if (m_egl_surface != NULL) {
        m_egl_surface->Release();
        delete m_egl_surface;
        m_egl_surface = NULL;
    }
}

void GLRender::ReleaseWindow() {
    if (m_native_window != NULL) {
        ANativeWindow_release(m_native_window);
        m_native_window = NULL;
    }
}


void GLRender::SetBitmapRender(ImageRender *bitmapRender) {
    pImageRender = bitmapRender;
}

ImageRender *GLRender::GetImageRender() {
    return pImageRender;
}




