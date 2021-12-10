#ifndef EGL_COMMON_H
#define EGL_COMMON_H

#include "gl_common.h"
#include "common.h"

#define EGL_OPENGL_ES3_BIT_KHR            0x00000040

typedef struct
{
    EGLNativeWindowType win;
    EGLDisplay  eglDisplay;
    /// EGL context
    EGLContext  eglContext;
    /// EGL surface
    EGLSurface  eglSurface;
    EGLConfig config;
    EGLint numConfigs;
    //共享上下文 Context
    EGLContext _sharedContext = EGL_NO_CONTEXT; 
    EGLint majorVersion;
    EGLint minorVersion;

} _EGLContext;

class EGLCommon
{
private:
    _EGLContext *eglContext;

public:
    EGLCommon();
    ~EGLCommon();
    int initEGL_EXT_EGL_GLEW();
    int initEGL_EXT_surfaceless();
    const char * PrintExtensions(const char *extensions);
    int initEGL(EGLNativeDisplayType display_id);
    int eglConfig(EGLint *attribList);
    int createSurface(EGLNativeWindowType win);
    int createPbuSurface();
    int createPixmipSurface();
    int createContext(EGLint *contextAttribs,EGLContext sharedContext);
    int makeCurrent();
    int swapBuffer();
};

#endif