#include "view.h"

View::View()
{
    //C++14引入
    x11 = std::unique_ptr<X11>(new X11());
    egl = std::unique_ptr<EGLCommon>(new EGLCommon());
    x11Context = (X11Context *)malloc(sizeof(X11Context));
    glcontext = (GLContext *)malloc(sizeof(GLContext));
}

View::~View()
{
    SAFE_FREE(x11Context)
    SAFE_FREE(glcontext)
}

int View::init(ViewContext *viewContext)
{
    _viewContext = viewContext;
    x11Context->width = viewContext->width;
    x11Context->height = viewContext->height;
    glcontext->width = viewContext->width;
    glcontext->height = viewContext->height;
    return 0;
}

int View::createWin()
{
    // x11.get()->WinCreate(x11Context, _viewContext->title.c_str());
    return 0;
}

int View::configegl(EGLNativeDisplayType display_id)
{

    // if (egl.get()->initEGL((display_id != nullptr) ? display_id : x11Context->eglNativeDisplay))
    // {
    //     LogOut("init egl falied");
    //     return -1;
    // }

    // // if (egl.get()->initEGL_EXT())
    // // {
    // //     LogOut("init egl falied");
    // //     return -1;
    // // }

    // if (egl.get()->eglConfig(nullptr))
    // {
    //     LogOut("egl config failed");
    //     return -1;
    // }
    // if (egl.get()->createSurface(x11Context->eglNativeWindow))
    // {
    //     LogOut("create pbu surface window failed");
    //     return -1;
    // }
    // EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    // if (egl.get()->createContext(contextAttribs, EGL_NO_CONTEXT))
    // {
    //     LogOut("create egl context failed!");
    //     return -1;
    // }
    // if (egl.get()->makeCurrent())
    // {
    //     LogOut("egl bind failed");
    //     return -1;
    // }

    //使用 EGL + GLEW 方式
    egl.get()->initEGL_EXT_EGL_GLEW();

    return 0;
}

int View::configGL()
{
    if (vertexSrc.empty())
    {
        vertexSrc = VEXTER_SHADER;
    }
    if (coordSrc.empty())
    {
        coordSrc = FRAGMENT_SHADER;
    }
    if (GLUtil::loadProgram(glcontext, vertexSrc.c_str(), coordSrc.c_str()))
    {
        LogOut("laod opengl program failed");
        return -1;
    }
    return 0;
}

int View::draw()
{
    GLUtil::drawBefore(glcontext);
    glViewport(0, 0, 544, 960);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(0);
    //本地向CPU 传输
    // glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    FILE *fd = fopen("/home/ubuntu/lzl/source/opengl_test_hc3/build/test.rgba","wb");
    uint8_t *data = new uint8_t[544*960*4]; 
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glFlush();
    glReadPixels(0, 0,544, 960, GL_RGBA, GL_UNSIGNED_BYTE, data);
    fwrite(data,544*960*4,1,fd);
    fclose(fd);

    return 0;
}

int View::swap()
{
    egl.get()->swapBuffer();
    return 0;
}

int View::destroy()
{

    return 0;
}