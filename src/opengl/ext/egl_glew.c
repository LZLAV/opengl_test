#define GL_TRANSITION_USING_EGL //remove this line if you don't want to use EGL
#include <GL/glew.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#define FROM (0)
#define TO (1)

#define PIXEL_FORMAT (GL_RGB)

#ifdef GL_TRANSITION_USING_EGL
static const EGLint configAttribs[] = {
    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
    EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE, 8,
    EGL_DEPTH_SIZE, 8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    EGL_NONE};
#endif
static const float position[12] = {
    -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f};

GLuint from;
GLuint to;
GLint progress;
GLint ratio;
GLint _fromR;
GLint _toR;
GLuint posBuf;
GLuint program;

EGLDisplay eglDpy;
EGLConfig eglCfg;
EGLSurface eglSurf;
EGLContext eglCtx;

static const GLchar *v_shader_source =
    "attribute vec2 position;\n"
    "varying vec2 _uv;\n"
    "void main(void) {\n"
    "  gl_Position = vec4(position, 0, 1);\n"
    "  vec2 uv = position * 0.5 + 0.5;\n"
    "  _uv = vec2(uv.x, 1.0 - uv.y);\n"
    "}\n";

static const GLchar *f_shader_template =
    "#version 300 es\n"
    "precision mediump float;\n"
    "out vec4 fragColor;\n"
    "void main()\n"
    "{\n"
      "fragColor = vec4(1.0,0.0,0.0,1.0);\n"
    "}\n";

// default to a basic fade effect
static const GLchar *f_default_transition_source =
    "vec4 transition (vec2 uv) {\n"
    "  return mix(\n"
    "    getFromColor(uv),\n"
    "    getToColor(uv),\n"
    "    progress\n"
    "  );\n"
    "}\n";

static GLuint build_shader(const GLchar *shader_source, GLenum type)
{
  GLuint shader = glCreateShader(type);
  if (!shader || !glIsShader(shader))
  {
    return 0;
  }

  glShaderSource(shader, 1, &shader_source, 0);
  glCompileShader(shader);

  GLint status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

  return (status == GL_TRUE ? shader : 0);
}

static int build_program()
{
  GLuint v_shader, f_shader;

  if (!(v_shader = build_shader(v_shader_source, GL_VERTEX_SHADER)))
  {
    return -1;
  }

  if (!(f_shader = build_shader(f_shader_template, GL_FRAGMENT_SHADER)))
  {
    return -1;
  }

  program = glCreateProgram();
  glAttachShader(program, v_shader);
  glAttachShader(program, f_shader);
  glLinkProgram(program);

  GLint status;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  return status == GL_TRUE ? 0 : -1;
}

static void setup_vbo()
{
  glGenBuffers(1, &posBuf);
  glBindBuffer(GL_ARRAY_BUFFER, posBuf);
  glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);

  GLint loc = glGetAttribLocation(program, "position");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
}

static void setup_tex()
{
  { // from
    glGenTextures(1, &from);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, from);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 544, 960, 0, PIXEL_FORMAT, GL_UNSIGNED_BYTE, NULL);

    glUniform1i(glGetUniformLocation(program, "from"), 0);
  }
}

static void setup_uniforms()
{

  progress = glGetUniformLocation(program, "progress");
  glUniform1f(progress, 0.0f);

  // TODO: this should be output ratio
  ratio = glGetUniformLocation(program, "ratio");
  glUniform1f(ratio, 544 / 960);

  _fromR = glGetUniformLocation(program, "_fromR");
  glUniform1f(_fromR, 544 / 960);

  // TODO: initialize this in config_props for "to" input
  _toR = glGetUniformLocation(program, "_toR");
  glUniform1f(_toR, 544 / 960);
}

static int setup_gl()
{
  //init EGL
  // 1. Initialize EGL
  // c->eglDpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);

#define MAX_DEVICES 4
  EGLDeviceEXT eglDevs[MAX_DEVICES];
  EGLint numDevices;

  PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT = (PFNEGLQUERYDEVICESEXTPROC)
      eglGetProcAddress("eglQueryDevicesEXT");

  eglQueryDevicesEXT(MAX_DEVICES, eglDevs, &numDevices);

  PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC)
      eglGetProcAddress("eglGetPlatformDisplayEXT");

  eglDpy = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, eglDevs[0], 0);

  EGLint major, minor;
  eglInitialize(eglDpy, &major, &minor);
  // 2. Select an appropriate configuration
  EGLint numConfigs;
  EGLint pbufferAttribs[] = {
      EGL_WIDTH,
      544,
      EGL_HEIGHT,
      960,
      EGL_NONE,
  };
  eglChooseConfig(eglDpy, configAttribs, &eglCfg, 1, &numConfigs);
  // 3. Create a surface
  eglSurf = eglCreatePbufferSurface(eglDpy, eglCfg,
                                    pbufferAttribs);
  // 4. Bind the API
  eglBindAPI(EGL_OPENGL_API);
  // 5. Create a context and make it current
  eglCtx = eglCreateContext(eglDpy, eglCfg, EGL_NO_CONTEXT, NULL);
  eglMakeCurrent(eglDpy, eglSurf, eglSurf, eglCtx);

  glewExperimental = GL_TRUE;
  glewInit();

  glViewport(0, 0, 544, 960);

  int ret;
  if ((ret = build_program()) < 0)
  {
    return ret;
  }

  glUseProgram(program);
  setup_vbo();
  setup_uniforms();
  setup_tex();

  return 0;
}

static void apply_transition()
{

#ifdef GL_TRANSITION_USING_EGL
  eglMakeCurrent(eglDpy, eglSurf, eglSurf, eglCtx);
#else
  glfwMakeContextCurrent(c->window);
#endif

  glUseProgram(program);

  // av_log(ctx, AV_LOG_ERROR, "transition '%s' %llu %f %f\n", c->source, fs->pts - c->first_pts, ts, progress);
  glUniform1f(progress, 0.2);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, from);
  //   glPixelStorei(GL_UNPACK_ROW_LENGTH, fromFrame->linesize[0] / 3);
  //   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fromLink->w, fromLink->h, 0, PIXEL_FORMAT, GL_UNSIGNED_BYTE, fromFrame->data[0]);

  //   glActiveTexture(GL_TEXTURE0 + 1);
  //   glBindTexture(GL_TEXTURE_2D, c->to);
  //   glPixelStorei(GL_UNPACK_ROW_LENGTH, toFrame->linesize[0] / 3);
  //   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, toLink->w, toLink->h, 0, PIXEL_FORMAT, GL_UNSIGNED_BYTE, toFrame->data[0]);

  //   glDrawArrays(GL_TRIANGLES, 0, 6);
  //   glPixelStorei(GL_PACK_ROW_LENGTH, outFrame->linesize[0] / 3);
  //   glReadPixels(0, 0,544, 960, PIXEL_FORMAT, GL_UNSIGNED_BYTE, (GLvoid *)outFrame->data[0]);

  glPixelStorei(GL_PACK_ROW_LENGTH, 0);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}

static void uninit()
{

  if (eglDpy)
  {
    glDeleteTextures(1, &from);
    // glDeleteTextures(1, &c->to);
    glDeleteBuffers(1, &posBuf);
    glDeleteProgram(program);
    eglTerminate(eglDpy);
  }
}


int main (int argc,char **argv)
{
    setup_gl();
    apply_transition();
    return 0;
}