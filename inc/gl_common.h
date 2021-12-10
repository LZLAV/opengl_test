#ifndef GL_COMMON_H
#define GL_COMMON_H

#include <GL/glew.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
// #include <GLES3/gl3.h>

typedef struct
{
    int width;
    int height;
    unsigned int flag;    //clear flag
    GLuint programObject;
} GLContext;

#endif