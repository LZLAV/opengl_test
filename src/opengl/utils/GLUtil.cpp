#include "GLUtil.h"

GLuint GLUtil::loadShader(GLenum type, const char *shaderSrc)
{
    GLuint shader;
    GLint compiled;

    // Create the shader object
    shader = glCreateShader(type);

    if (shader == 0)
    {
        return 0;
    }

    // Load the shader source
    glShaderSource(shader, 1, &shaderSrc, NULL);

    // Compile the shader
    glCompileShader(shader);

    // Check the compile status
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled)
    {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 1)
        {
            char *infoLog = (char *)malloc(sizeof(char) * infoLen);

            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            std::cout << "Error compiling shader:\n"
                      << infoLog << std::endl;
            free(infoLog);
        }

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint GLUtil::loadProgram(GLContext *context,const char *vertShaderSrc, const char *fragShaderSrc)
{
    GLuint vertexShader;
    GLuint fragmentShader;
    GLint linked;

    // Load the vertex/fragment shaders
    vertexShader = loadShader(GL_VERTEX_SHADER, vertShaderSrc);

    if (vertexShader == 0)
    {
        return -1;
    }

    fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragShaderSrc);

    if (fragmentShader == 0)
    {
        glDeleteShader(vertexShader);
        return -1;
    }

    // Create the program object
    context->programObject = glCreateProgram();

    if (context->programObject == 0)
    {
        return -1;
    }

    glAttachShader(context->programObject, vertexShader);
    glAttachShader(context->programObject, fragmentShader);

    // Link the program
    glLinkProgram(context->programObject);

    // Check the link status
    glGetProgramiv(context->programObject, GL_LINK_STATUS, &linked);

    if (!linked)
    {
        showGLErrorInfo(context->programObject,__func__);
        glDeleteProgram(context->programObject);
        return -1;
    }

    // Free up no longer needed shader resources
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return 0;
}

void GLUtil::showGLErrorInfo(GLuint program,const char *func)
{
    GLint infoLen = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
    if (infoLen > 1)
    {
        char *infoLog = (char *)malloc(sizeof(char) * infoLen);

        glGetProgramInfoLog(program, infoLen, NULL, infoLog);
        char *str;
        sprintf(str, "%s,Error linking program:\n%s\n",func,infoLog);
        LogOut(str);
        free(infoLog);
    }
}

void GLUtil::drawBefore(GLContext *glcontext)
{
    glClearColor ( 1.0f, 1.0f, 1.0f, 0.0f );
    glViewport ( 0, 0, glcontext->width, glcontext->height);
    glClear ( glcontext->flag );
    glUseProgram (glcontext->programObject);
}