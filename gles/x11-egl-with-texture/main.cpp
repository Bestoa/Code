#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#include <GLES3/gl32.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include  <X11/Xlib.h>
#include  <X11/Xatom.h>
#include  <X11/Xutil.h>

#define USE_GBM_TEX 1

#if USE_GBM_TEX
#include <gbm.h>
#include <xf86drm.h>
#include <drm_fourcc.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// X11 related local variables
static Display *x_display = NULL;
static Atom s_wmDeleteMessage;

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

#if !USE_GBM_TEX
bool loadTexture(const char *name, GLuint &tex_id)
{
    //stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char *data = stbi_load(name, &width, &height, &nrChannels, 0);

    if (data == nullptr)
    {
        printf("Load texture Failed.\n");
        return false;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    printf("w = %d height = %d nrChannels = %d\n", width, height, nrChannels);

    if (nrChannels == 3)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    else if (nrChannels == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    else
    {
        printf("Only support RGB888/RGBA8888 texture.\n");
        goto error;
    }
    if (glGetError() != GL_NO_ERROR)
        goto error;

    stbi_image_free(data);
    tex_id = texture;

    return true;
error:
    glDeleteTextures(1, &texture);
    return false;
}
#endif

#if USE_GBM_TEX
constexpr int GBM_BUFFER_W = 500;
constexpr int GBM_BUFFER_H = 500;
#endif

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

#if !USE_GBM_TEX
    GLuint tex_id;
    loadTexture("tex.tga", tex_id);
#else
    int drm_fd = open("/dev/dri/card1", O_RDWR | O_CLOEXEC);
    assert(drm_fd >= 0);
    struct gbm_device* gbm = gbm_create_device(drm_fd);
    assert(gbm != nullptr);
    struct gbm_bo *gbo = gbm_bo_create(gbm, GBM_BUFFER_W, GBM_BUFFER_H, GBM_FORMAT_XBGR8888, GBM_BO_USE_LINEAR);
    assert(gbo != nullptr);
    int gbm_dmabuf_fd = gbm_bo_get_fd(gbo);
    assert(gbm_dmabuf_fd >= 0);
    int gbo_pitch = gbm_bo_get_stride(gbo);
    printf("gbo pitch = %d\n", gbo_pitch);
    void *vaddr;
    vaddr = mmap(nullptr, gbo_pitch * GBM_BUFFER_W, (PROT_READ | PROT_WRITE), MAP_SHARED, gbm_dmabuf_fd, 0);
    printf("gbo vaddr = %p\n", vaddr);
    for (int i = 0; i < gbo_pitch * GBM_BUFFER_H; i ++)
    {
        if (i % 4 == 0)
            *reinterpret_cast<uint8_t *>(vaddr + i) = 255;
        else
            *reinterpret_cast<uint8_t *>(vaddr + i) = 0;
    }
    munmap(vaddr, gbo_pitch * GBM_BUFFER_H);

    PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
    assert(eglCreateImageKHR != nullptr);
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
    assert(glEGLImageTargetTexture2DOES != nullptr);
    PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
    assert(eglDestroyImageKHR != nullptr);

    const EGLint khr_image_attrs[] = {
        EGL_DMA_BUF_PLANE0_FD_EXT, gbm_dmabuf_fd,
        EGL_WIDTH, GBM_BUFFER_W,
        EGL_HEIGHT, GBM_BUFFER_H,
        EGL_LINUX_DRM_FOURCC_EXT, GBM_FORMAT_XBGR8888,
        EGL_DMA_BUF_PLANE0_PITCH_EXT, gbo_pitch,
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
        EGL_NONE, EGL_NONE
    };

    EGLImage eglImage = eglCreateImageKHR(eglDisplay, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, nullptr /* no client buffer */, khr_image_attrs);

    GLuint dma_tex_id;
    glGenTextures(1, &dma_tex_id);
    glBindTexture(GL_TEXTURE_2D, dma_tex_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, eglImage);
    checkGlError("glEGLImageTargetTexture2DOES");
#endif
    shader.setInt("tex", 0);

    while(userInterrupt() == GL_FALSE)
    {
        glClearColor(0.00f, 0.70f, 0.67f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        eglSwapBuffers(eglDisplay, eglSurface);
    }

#if USE_GBM_TEX
    eglDestroyImageKHR(eglDisplay, eglImage);
    glDeleteTextures(1, &dma_tex_id);
    gbm_bo_destroy(gbo);
    gbm_device_destroy(gbm);
    close(drm_fd);
#else
    glDeleteTextures(1, &tex_id);
#endif
    glDeleteBuffers(1, &mVBO);
    eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglTerminate(eglDisplay);

    return 0;
}
