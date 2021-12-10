#ifndef X11_H
#define X11_H

#include  <X11/Xlib.h>
#include  <X11/Xatom.h>
#include  <X11/Xutil.h>

#include "common.h"
#include "gl_common.h"


typedef struct
{
    int width;
    int height;
    EGLNativeWindowType  eglNativeWindow;
    EGLNativeDisplayType eglNativeDisplay;
} X11Context;

class X11
{
private:


public:
    X11()= default;
    ~X11()=default;
    int WinCreate(X11Context *x11Context, const char *title);

};



#endif