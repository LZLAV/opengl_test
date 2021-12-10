#include "X11.h"

/**
 * @brief 获取X11 window 和 display
 * 
 * @param x11Context 
 * @param title 
 * @return int 
 */
int X11::WinCreate(X11Context *x11Context, const char *title)
{
    static Display *x_display = NULL;
    Window root;
    XSetWindowAttributes swa;
    XSetWindowAttributes  xattr;
    Atom wm_state;
    XWMHints hints;
    XEvent xev;
    Window win;
    static Atom s_wmDeleteMessage;

    /*
     * X11 native display initialization
     */

    x_display = XOpenDisplay(NULL);
    if ( x_display == NULL )
    {
        return -1;
    }

    root = DefaultRootWindow(x_display);

    swa.event_mask  =  ExposureMask | PointerMotionMask | KeyPressMask;
    win = XCreateWindow(
               x_display, root,
               0, 0, x11Context->width, x11Context->height, 0,
               CopyFromParent, InputOutput,
               CopyFromParent, CWEventMask,
               &swa );
    s_wmDeleteMessage = XInternAtom(x_display, "WM_DELETE_WINDOW", 0);
    XSetWMProtocols(x_display, win, &s_wmDeleteMessage, 1);

    xattr.override_redirect = 0;
    XChangeWindowAttributes ( x_display, win, CWOverrideRedirect, &xattr );

    hints.input = 1;
    hints.flags = InputHint;
    XSetWMHints(x_display, win, &hints);

    // make the window visible on the screen
    XMapWindow (x_display, win);
    XStoreName (x_display, win, title);

    // get identifiers for the provided atom name strings
    wm_state = XInternAtom (x_display, "_NET_WM_STATE", 0);

    memset (&xev, 0, sizeof(xev));
    xev.type                 = ClientMessage;
    xev.xclient.window       = win;
    xev.xclient.message_type = wm_state;
    xev.xclient.format       = 32;
    xev.xclient.data.l[0]    = 1;
    xev.xclient.data.l[1]    = 0;
    XSendEvent (
       x_display,
       DefaultRootWindow ( x_display ),
       0,
       SubstructureNotifyMask,
       &xev );

    x11Context->eglNativeWindow = (EGLNativeWindowType) win;
    x11Context->eglNativeDisplay = (EGLNativeDisplayType) x_display;
    return 0;
}