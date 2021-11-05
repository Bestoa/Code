#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstring>
#include <fcntl.h>

#include <GLES3/gl32.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include  <X11/Xlib.h>
#include  <X11/Xatom.h>
#include  <X11/Xutil.h>

#include <xf86drm.h>
#include <drm.h>
#include <drm_fourcc.h>
#include "buffers.h"

// X11 related local variables
static Display *x_display = NULL;
static Atom s_wmDeleteMessage;

PFNEGLCREATEIMAGEKHRPROC pfneglCreateImageKHR = NULL;
PFNEGLDESTROYIMAGEKHRPROC pfneglDestroyImageKHR = NULL;
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC pfnglEGLImageTargetTexture2DOES = NULL;

#include "shader.h"

using namespace std;
using namespace glm;

#define WIDTH (800)
#define HEIGHT (800)

bool testEGLError(const char* functionLastCalled)
{
    EGLint lastError = eglGetError();
    if (lastError != EGL_SUCCESS)
    {
        printf("%s failed (%x).\n", functionLastCalled, lastError);
        return false;
    }
    return true;
}

void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error
            = glGetError()) {
        printf("after %s() glError (0x%x)\n", op, error);
    }
}

EGLBoolean WinCreate(int w, int h, const char *title, EGLNativeDisplayType &eglNativeDisplay, EGLNativeWindowType &eglNativeWindow)
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
        return EGL_FALSE;
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
    XStoreName (x_display, win, title);

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

    eglNativeWindow = (EGLNativeWindowType) win;
    eglNativeDisplay = (EGLNativeDisplayType) x_display;
    return EGL_TRUE;
}

GLboolean userInterrupt()
{
    XEvent xev;
    GLboolean userinterrupt = GL_FALSE;

    while ( XPending ( x_display ) )
    {
        XNextEvent( x_display, &xev );
        if (xev.type == ClientMessage) {
            if (xev.xclient.data.l[0] == s_wmDeleteMessage) {
                userinterrupt = GL_TRUE;
            }
        }
        if ( xev.type == DestroyNotify )
            userinterrupt = GL_TRUE;
    }
    return userinterrupt;
}

void copy_yuyv(void *vaddr, void *data, int w, int h, int pitch)
{
    for (int i = 0; i < h; i++)
    {
        memcpy(vaddr, data, w*2);
        vaddr += pitch;
        data += w*2;
    }
}

int main()
{
    EGLNativeDisplayType eglNativeDisplay;
    EGLNativeWindowType eglNativeWindow;
    if (WinCreate(WIDTH, HEIGHT, "Test Window", eglNativeDisplay, eglNativeWindow) != EGL_TRUE)
    {
        printf("Failed to create window\n");
        return false;
    }
    EGLDisplay eglDisplay = eglGetDisplay(eglNativeDisplay);
    if (eglDisplay == EGL_NO_DISPLAY)
    {
        printf("Failed to get an EGLDisplay\n");
        return false;
    }

    EGLint eglMajorVersion = 0;
    EGLint eglMinorVersion = 0;
    if (!eglInitialize(eglDisplay, &eglMajorVersion, &eglMinorVersion))
    {
        printf("Failed to initialize the EGLDisplay\n");
        return false;
    }

    printf("EGL version: %d.%d\n", eglMajorVersion, eglMinorVersion);

    if (eglBindAPI(EGL_OPENGL_ES_API) != EGL_TRUE)
    {
        printf("Failed to bind egl api\n");
        return false;
    }
    EGLint configurationAttributes[] = {
        EGL_BUFFER_SIZE,    EGL_DONT_CARE,
        EGL_RENDERABLE_TYPE,    EGL_OPENGL_ES3_BIT,
        EGL_NONE
    };
    EGLint configsReturned;
    EGLConfig eglConfig;
    if (!eglChooseConfig(eglDisplay, configurationAttributes, &eglConfig, 1, &configsReturned) || (configsReturned != 1))
    {
        printf("Failed to choose a suitable config.");
        return false;
    }
    EGLSurface eglSurface;
    eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, (EGLNativeWindowType)eglNativeWindow, NULL);
    if (!testEGLError("eglCreateWindowSurface")) { return false; }

    eglBindAPI(EGL_OPENGL_ES_API);
    if (!testEGLError("eglBindAPI")) { return false; }

    EGLint contextAttributes[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 2,
        EGL_NONE
    };

    EGLContext context;
    context = eglCreateContext(eglDisplay, eglConfig, NULL, contextAttributes);
    if (!testEGLError("eglCreateContext")) { return false; }

    eglMakeCurrent(eglDisplay, eglSurface, eglSurface, context);
    if (!testEGLError("eglMakeCurrent")) { return false; }

    pfneglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
    if (pfneglCreateImageKHR == NULL) {
        printf("Failed to get func eglCreateImageKHR\n");
        return false;
    }
    pfneglDestroyImageKHR =
        (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
    if (pfneglDestroyImageKHR == NULL) {
        printf( "eglGetProcAddress failed for eglDestroyImageKHR\n");
        return false;
    }
    pfnglEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)
        eglGetProcAddress("glEGLImageTargetTexture2DOES");
    if (pfnglEGLImageTargetTexture2DOES == NULL) {
        printf("eglGetProcAddress failed for glEGLImageTargetTexture2DOES\n");
        return false;
    }

    Shader shader("quad.vert", "quad.frag");
    shader.use();
    GLfloat triangles[] =
    {
        -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
    };

    GLuint mVBO;
    glGenBuffers(1, &mVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, 4 * 6 * sizeof(GLfloat), triangles, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), reinterpret_cast<void *>(4*sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    int fd = open("/dev/dri/card1", O_RDWR, 0);
    if (fd < 0) {
        printf("Open DRM device failed\n");
        return false;
    }

#define IMG_WIDTH (600)
#define IMG_HEIGHT (600)
#define DATA_LEN_YUYV (IMG_WIDTH*IMG_HEIGHT*2)

    int data_len = DATA_LEN_YUYV;
    char *texname = "grid_yuyv_600.yuv";
    int format = DRM_FORMAT_YUYV;

    void *data = malloc(data_len);
    if (!data) {
        printf("Malloc texture data failed\n");
        return false;
    }

    FILE *fp = fopen(texname, "rb");
    if (!fp) {
        printf("Open texture file failed.\n");
        return false;
    }
    int read_bytes = fread(data, 1, data_len, fp);
    fclose(fp);
    if (read_bytes != data_len)
    {
        printf("read file error.\n");
        return false;
    }

    unsigned int handles[4] = { 0 };
    unsigned int pitches[4] = { 0 };
    unsigned int offsets[4] = { 0 };
    void *vaddr;

    struct bo *pbo = bo_create(fd, format, IMG_WIDTH, IMG_WIDTH, handles, pitches, offsets, &vaddr);
    if (!pbo)
    {
        printf("create bo failed.\n");
        return false;
    }
    for (int i = 0; i < 4; i++)
    {
        printf("handld pitch offset [i] = %d %d %d\n", handles[i], pitches[i], offsets[i]);
    }
    // For test, we need to load data.
    copy_yuyv(vaddr, data, IMG_WIDTH, IMG_HEIGHT, pitches[0]);

    struct drm_prime_handle prime;
    memset(&prime, 0, sizeof(prime));
    prime.handle = handles[0];
    if (drmIoctl(fd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &prime)) {
        printf("Failed to export fd (err=%d)\n", errno);
        return false;
    }

    EGLint aiImageAttribs[25] =
    {
        EGL_WIDTH, IMG_WIDTH,
        EGL_HEIGHT, IMG_HEIGHT,
        EGL_LINUX_DRM_FOURCC_EXT, format,
        EGL_DMA_BUF_PLANE0_FD_EXT, prime.fd,
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, offsets[0],
        EGL_DMA_BUF_PLANE0_PITCH_EXT, pitches[0],
        EGL_NONE,
    };
    EGLImage eglImage = pfneglCreateImageKHR(eglDisplay, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, aiImageAttribs);
    if (EGL_NO_IMAGE == eglImage)
    {
        printf( "Failed to create image\n");
        return false;
    }

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, tex);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    pfnglEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, eglImage);
    checkGlError("pfnglEGLImageTargetTexture2DOES");
    shader.setInt("yuvTexSampler", 0);

    while(userInterrupt() == GL_FALSE)
    {
        glClearColor(0.00f, 0.70f, 0.67f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        eglSwapBuffers(eglDisplay, eglSurface);
    }

    free(data);
    bo_destroy(pbo);
    pfneglDestroyImageKHR(eglDisplay, eglImage);
    glDeleteBuffers(1, &mVBO);
    eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglTerminate(eglDisplay);

    return 0;
}
