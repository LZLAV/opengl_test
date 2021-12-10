#ifndef APP_H
#define APP_H

#define EGL_EGLEXT_PROTOTYPES

#include "common.h"
#include "gl_common.h"
#include "view.h"
#include <assert.h>

class App
{

private:
    

    ViewContext *viewContext;
    View *_view;

public:
    App();
    ~App();
    int init();
    int draw();
};

#endif