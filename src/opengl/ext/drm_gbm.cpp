
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>
#include <EGL/egl.h>
#include <GL/gl.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

// gcc -g -o SimpleGBMDemo SimpleGBMDemo.c -ldrm -lgbm -lEGL -lGL -I/usr/include/libdrm
// https://blog.csdn.net/eydwyz/article/details/107046470	程序解释
 
#define EXIT(msg) { fputs (msg, stderr); exit (EXIT_FAILURE); }
 
void assertEGLError(const char *msg)
{
    EGLint error = eglGetError();
    if(error != EGL_SUCCESS) {
        printf("EGL error 0x%x at %s\n", error, msg);
    }
}
 
static int g_device_fd;
 
static uint32_t connector_id;
static drmModeModeInfo mode_info;
static drmModeCrtc *crtc;
static struct gbm_device *gbm_device;
 
static EGLDisplay display;
static EGLContext context;
static struct gbm_surface *gbm_surface;
static EGLSurface egl_surface;
 
static struct gbm_bo *previous_bo = NULL;
static uint32_t previous_fb;
 
static void swap_buffers () {
	eglSwapBuffers (display, egl_surface);
	struct gbm_bo *bo = gbm_surface_lock_front_buffer (gbm_surface);
	uint32_t handle = gbm_bo_get_handle (bo).u32;
	uint32_t pitch = gbm_bo_get_stride (bo);
	uint32_t fb;
	drmModeAddFB (g_device_fd, mode_info.hdisplay, mode_info.vdisplay, 24, 32, pitch, handle, &fb);
	drmModeSetCrtc (g_device_fd, crtc->crtc_id, fb, 0, 0, &connector_id, 1, &mode_info);
	
	if (previous_bo) {
		drmModeRmFB (g_device_fd, previous_fb);
		gbm_surface_release_buffer (gbm_surface, previous_bo);
	}
	previous_bo = bo;
	previous_fb = fb;
}
 
static void draw (float progress) {
	glClearColor (1.0f-progress, progress, 0.0, 1.0);
	glClear (GL_COLOR_BUFFER_BIT);
	swap_buffers ();
}
 
static void clean_up () {
	// set the previous crtc
	drmModeSetCrtc (g_device_fd, crtc->crtc_id, crtc->buffer_id, crtc->x, crtc->y, &connector_id, 1, &crtc->mode);
	drmModeFreeCrtc (crtc);
	
	if (previous_bo) {
		drmModeRmFB (g_device_fd, previous_fb);
		gbm_surface_release_buffer (gbm_surface, previous_bo);
	}
	
	eglDestroySurface (display, egl_surface);
	gbm_surface_destroy (gbm_surface);
	eglDestroyContext (display, context);
	eglTerminate (display);
	gbm_device_destroy (gbm_device);
}
 
/* On-Screen Rendering */
void print_default_display_info()
{
	int major, minor;
    EGLDisplay defaultDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    assertEGLError("eglGetDisplay(EGL_DEFAULT_DISPLAY)");
    eglInitialize (defaultDisplay, &major, &minor);
    assertEGLError("eglInitialize");
    printf("*************EGL %d.%d when eglGetDisplay(EGL_DEFAULT_DISPLAY)********t*****\n", major, minor);
	printf(" EGL_CLIENT_APIS: %s\n", eglQueryString(defaultDisplay, EGL_CLIENT_APIS));
	printf(" EGL_VENDOR: %s\n", eglQueryString(defaultDisplay,  EGL_VENDOR));
	printf(" EGL_VERSION: %s\n", eglQueryString(defaultDisplay,  EGL_VERSION));
	printf(" EGL_EXTENSIONS: %s\n", eglQueryString(defaultDisplay,  EGL_EXTENSIONS));
    eglTerminate (defaultDisplay);
    assertEGLError("eglTerminate");
    printf("\n");
}
 
static void find_display_configuration () 
{
	drmModeRes *resources = drmModeGetResources (g_device_fd);
	/* It will crash if GPU driver doesn't support DRM/DRI. */
	assert(!(resources == NULL));
	
	// find a connector
	drmModeConnector *connector = NULL;
	for (int i=0; i < resources->count_connectors; i++) {
		drmModeConnector *temp_connector = drmModeGetConnector (g_device_fd, resources->connectors[i]);
		// pick the first connected connector
		if (temp_connector->connection == DRM_MODE_CONNECTED) {
			connector = temp_connector;
			break;
		}
		drmModeFreeConnector (connector);
	}
	if (!connector) EXIT ("no connector found\n");
	
	// save the connector_id
	connector_id = connector->connector_id;
	// save the first mode
	mode_info = connector->modes[0];
	printf ("resolution: %ix%i\n", mode_info.hdisplay, mode_info.vdisplay);
	
	// find an encoder
	drmModeEncoder *encoder = NULL;
	if (connector->encoder_id) {
		encoder = drmModeGetEncoder (g_device_fd, connector->encoder_id);
	}
	
	if (!encoder) EXIT ("no encoder found\n");
	// find a CRTC
	if (encoder->crtc_id) {
		crtc = drmModeGetCrtc (g_device_fd, encoder->crtc_id);
	}
	
	// clean up
	drmModeFreeEncoder (encoder);
	drmModeFreeConnector (connector);
	drmModeFreeResources (resources);
}
 
int main () 
{
	/* For comparing with Off-Screen Rendering Info */
	print_default_display_info();
	
	/* Off-Screen Rendering */
	printf("************/dev/dri/card0************\n");
	g_device_fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC | O_NOCTTY | O_NONBLOCK);
	
	find_display_configuration ();
	
	/* Create EGL Context using GBM */
	int major, minor;
	gbm_device = gbm_create_device (g_device_fd);
	display = eglGetDisplay (gbm_device);
	eglInitialize (display, &major, &minor);
	
    printf("*************EGL %d.%d when eglGetDisplay(GBM)*************\n", major, minor);
	printf(" EGL_CLIENT_APIS: %s\n", eglQueryString(display, EGL_CLIENT_APIS));
	printf(" EGL_VENDOR: %s\n", eglQueryString(display,  EGL_VENDOR));
	printf(" EGL_VERSION: %s\n", eglQueryString(display,  EGL_VERSION));
	printf(" EGL_EXTENSIONS: %s\n", eglQueryString(display,  EGL_EXTENSIONS));
	
	eglBindAPI (EGL_OPENGL_API);
	EGLint attributes[] = {
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_NONE };
	EGLConfig config;
	EGLint num_config;
	eglChooseConfig (display, attributes, &config, 1, &num_config);
	context = eglCreateContext (display, config, EGL_NO_CONTEXT, NULL);
	
	// create the GBM and EGL surface
	gbm_surface = gbm_surface_create (gbm_device, mode_info.hdisplay, mode_info.vdisplay, 
									GBM_BO_FORMAT_XRGB8888, GBM_BO_USE_SCANOUT|GBM_BO_USE_RENDERING);
	egl_surface = eglCreateWindowSurface (display, config, gbm_surface, NULL);
	eglMakeCurrent (display, egl_surface, egl_surface, context);
	
	/* Rendering using OpenGL... */
	int i;
	for (i = 0; i < 600; i++)
		draw (i / 600.0f);
	
	clean_up ();
	close (g_device_fd);
	
	return 0;
}