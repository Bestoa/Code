#include <cstdio>
#include <cstring>

#include <EGL/egl.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

// X11 related local variables
static Display *x_display = NULL;
static Atom s_wmDeleteMessage;
static bool gShouldStop = false;

bool nativeInit(int w, int h, EGLNativeDisplayType *eglNativeDisplay, EGLNativeWindowType *eglNativeWindow)
{
    Window root;
    XSetWindowAttributes swa;
    XSetWindowAttributes  xattr;
    Atom wm_state;
    XWMHints hints;
    XEvent xev;
    Window win;

    /*
     * X11 native display initialization
     */

    x_display = XOpenDisplay(NULL);
    if ( x_display == NULL )
    {
        printf("Failed to open x display\n");
        return false;
    }

    root = DefaultRootWindow(x_display);

    swa.event_mask  =  ExposureMask | PointerMotionMask | KeyPressMask;
    win = XCreateWindow(
            x_display, root,
            0, 0, w, h, 0,
            CopyFromParent, InputOutput,
            CopyFromParent, CWEventMask,
            &swa );
    s_wmDeleteMessage = XInternAtom(x_display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(x_display, win, &s_wmDeleteMessage, 1);

    xattr.override_redirect = false;
    XChangeWindowAttributes ( x_display, win, CWOverrideRedirect, &xattr );

    hints.input = true;
    hints.flags = InputHint;
    XSetWMHints(x_display, win, &hints);

    // make the window visible on the screen
    XMapWindow (x_display, win);
    XStoreName (x_display, win, "X11 EGL demo");

    // get identifiers for the provided atom name strings
    wm_state = XInternAtom (x_display, "_NET_WM_STATE", false);

    memset ( &xev, 0, sizeof(xev) );
    xev.type                 = ClientMessage;
    xev.xclient.window       = win;
    xev.xclient.message_type = wm_state;
    xev.xclient.format       = 32;
    xev.xclient.data.l[0]    = 1;
    xev.xclient.data.l[1]    = false;
    XSendEvent (
            x_display,
            DefaultRootWindow ( x_display ),
            false,
            SubstructureNotifyMask,
            &xev );

    *eglNativeDisplay = (EGLNativeDisplayType) x_display;
    *eglNativeWindow = (EGLNativeWindowType) win;
    return true;
}

void nativeDestroy()
{
    XCloseDisplay(x_display);
}

void pollEvent()
{
    XEvent xev;
    while ( XPending ( x_display ) )
    {
        XNextEvent( x_display, &xev );
        if (xev.type == ClientMessage) {
            if ((Atom)xev.xclient.data.l[0] == s_wmDeleteMessage) {
                gShouldStop = true;
            }
        }
        if ( xev.type == DestroyNotify )
            gShouldStop = true;
    }
}

bool shouldStop()
{
    return gShouldStop;
}
