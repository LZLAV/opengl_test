#ifndef VIEW_H
#define VIEW_H

#include "gl_common.h"
#include "X11.h"
#include "GLUtil.h"
#include "eglCommon.h"
#include "common.h"

typedef struct
{
    int width;
    int height;
    std::string title;

} ViewContext;

class View
{
private:
    //唯一享有，不能赋值，可移动，reset() 重置，release() 释放所有权
    std::unique_ptr<X11> x11;    //win
    X11Context *x11Context;
    ViewContext *_viewContext;
    std::unique_ptr<EGLCommon>  egl;
    GLContext *glcontext;
    std::string vertexSrc;
    std::string coordSrc;

    const std::string VEXTER_SHADER = R"(
        #version 300 es
        layout(location = 0) in vec4 vPosition;
        void main()
        {
            gl_Position = vPosition;
        }
    )";

    const std::string FRAGMENT_SHADER = R"(
        #version 300 es
        precision mediump float;
        out vec4 fragColor;
        void main()
        {
            fragColor = vec4(1.0,0.0,0.0,1.0);
        }
    )";

    GLfloat vertices[9]={
        0.0f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f
    };

public:
    View();
    ~View();
    int init(ViewContext *viewContext);
    int createWin();
    int configegl(EGLNativeDisplayType display_id);
    int configGL();
    int draw();
    int swap();
    int destroy();
};

#endif