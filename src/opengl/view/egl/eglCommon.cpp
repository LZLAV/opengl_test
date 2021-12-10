#include "eglCommon.h"

EGLCommon::EGLCommon()
{
    eglContext = (_EGLContext*)malloc(sizeof(_EGLContext));
}

EGLCommon::~EGLCommon()
{
    SAFE_FREE(eglContext)
}

const char * EGLCommon::PrintExtensions(const char *extensions)
{
   const char *p, *end, *next;
   int column;

   column = 0;
   end = extensions + strlen(extensions);

   for (p = extensions; p < end; p = next + 1) {
      next = strchr(p, ' ');
      if (next == NULL)
         next = end;

      if (column > 0 && column + next - p + 1 > 70) {
	 printf("\n");
	 column = 0;
      }
      if (column == 0)
	 printf("    ");
      else
	 printf(" ");
      column += next - p + 1;

      printf("%.*s", (int) (next - p), p);

      p = next + 1;
   }

   if (column > 0)
      printf("\n");

   return extensions;
}

int EGLCommon::initEGL_EXT_surfaceless()
{
    const char *extensions;
    extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (!extensions)
        return NULL;
    extensions = PrintExtensions(extensions);

    if (strstr(extensions, "EGL_EXT_platform_base"))
    {
        PFNEGLGETPLATFORMDISPLAYEXTPROC getPlatformDisplay = (PFNEGLGETPLATFORMDISPLAYEXTPROC) eglGetProcAddress("eglGetPlatformDisplayEXT");
        if (strstr(extensions, "EGL_MESA_platform_surfaceless"))
        {
            eglContext->eglDisplay = getPlatformDisplay(EGL_PLATFORM_SURFACELESS_MESA,EGL_DEFAULT_DISPLAY,NULL);

            if (!eglInitialize(eglContext->eglDisplay, &eglContext->majorVersion, &eglContext->minorVersion)) 
            {
                printf("eglinfo: eglInitialize failed\n\n");
                return 1;
            }
        }
    }
}

int EGLCommon::initEGL_EXT_EGL_GLEW()
{
    EGLDisplay eglDpy;
    EGLConfig eglCfg;
    EGLSurface eglSurf;
    EGLContext eglCtx;

    EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };

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
}

int EGLCommon::initEGL(EGLNativeDisplayType display_id)
{
    eglContext->eglDisplay = eglGetDisplay(display_id);
    if (eglContext->eglDisplay == EGL_NO_DISPLAY)
    {
        return -1;
    }

    if (!eglInitialize(eglContext->eglDisplay, &eglContext->majorVersion, &eglContext->minorVersion))
    {
        return -1;
    }
    return 0;
}

int EGLCommon::eglConfig(EGLint *attribList)
{
    if (attribList == nullptr)
    {
        attribList = new EGLint[13]{
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_DEPTH_SIZE, 8,
            EGL_STENCIL_SIZE, 8,
            EGL_NONE};
    }
    // Choose config
    if (!eglChooseConfig(eglContext->eglDisplay, attribList, &eglContext->config, 1, &eglContext->numConfigs))
    {
        return -1;
    }

    if (eglContext->numConfigs < 1)
    {
        return -1;
    }
    return 0;
}

int EGLCommon::createSurface(EGLNativeWindowType win)
{
    // Create a surface
    eglContext->eglSurface = eglCreateWindowSurface(eglContext->eglDisplay, eglContext->config,
                                                    win, NULL);

    if (eglContext->eglSurface == EGL_NO_SURFACE)
    {
        return -1;
    }
    return 0;
}

int EGLCommon::createPbuSurface()
{
    eglContext->eglSurface = eglCreatePbufferSurface(eglContext->eglDisplay, eglContext->config,NULL);

    if (eglContext->eglSurface == EGL_NO_SURFACE)
    {
        return -1;
    }
    return 0;
}

int EGLCommon::createPixmipSurface()
{
}

int EGLCommon::createContext(EGLint *contextAttribs,EGLContext sharedContext)
{
    // Create a GL context
    eglContext->eglContext = eglCreateContext(eglContext->eglDisplay, eglContext->config,sharedContext,contextAttribs);

    if (eglContext->eglContext == EGL_NO_CONTEXT)
    {
        return -1;
    }
    return 0;
}

int EGLCommon::makeCurrent()
{
    if (!eglMakeCurrent(eglContext->eglDisplay, eglContext->eglSurface,
                        eglContext->eglSurface, eglContext->eglContext))
    {
        return -1;
    }
    return 0;
}

int EGLCommon::swapBuffer()
{
    eglSwapBuffers(eglContext->eglDisplay, eglContext->eglSurface); 
    return 0;
}