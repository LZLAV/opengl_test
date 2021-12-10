#ifndef GLUTIL_H
#define GLUTIL_H

#include "common.h"
#include "gl_common.h"

class GLUtil
{
private:


public:
    static GLuint loadShader(GLenum type,const char *shaderSrc);
    static GLuint loadProgram (GLContext *context,const char *vertShaderSrc, const char *fragShaderSrc);
    static void showGLErrorInfo(GLuint program,const char *func);
    static void drawBefore(GLContext *glcontext);
};



#endif