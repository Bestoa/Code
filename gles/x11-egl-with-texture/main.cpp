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

#define USE_DMABUF 1

#if USE_DMABUF
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

bool loadImageData(const char *name, unsigned char **addr, int *width, int *height)
{
    //stbi_set_flip_vertically_on_load(true);
    int nrChannels;
    // Force stbi return 4 channels.
    *addr = stbi_load(name, width, height, &nrChannels, 4);

    if (*addr == nullptr)
    {
        printf("Load texture Failed.\n");
        return false;
    }
    return true;
}

void freeImageData(unsigned char *addr)
{
    stbi_image_free(addr);
}

#if USE_DMABUF
void loadImageToGBMBUF(struct gbm_bo *gbo, uint8_t *data, int w, int h)
{
    int fd = gbm_bo_get_fd(gbo);
    assert(fd >= 0);
    int pitch = gbm_bo_get_stride(gbo);
    void *vaddr = mmap(nullptr, pitch * h, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, 0);
    uint8_t *dst, *src;
    for (int i = 0; i < h; i++)
    {
        src = data + i * w * 4;
        dst = (uint8_t *)(vaddr + i * pitch);
        for(int j = 0; j < pitch; j += 4)
        {
            dst[j + 0] = src[j + 0];
            dst[j + 1] = src[j + 1];
            dst[j + 2] = src[j + 2];
            dst[j + 3] = src[j + 3];
        }
    }
    munmap(vaddr, pitch * h);
}

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

    int width, height;
    uint8_t *data;
    assert(loadImageData("tex.tga", &data, &width, &height) == true);
    printf("Use GBM_TEX tex = %d\n", USE_DMABUF);
#if USE_DMABUF
    int drm_fd = open("/dev/dri/card1", O_RDWR | O_CLOEXEC);
    assert(drm_fd >= 0);
    struct gbm_device* gbm = gbm_create_device(drm_fd);
    assert(gbm != nullptr);
    struct gbm_bo *gbo = gbm_bo_create(gbm, width, height, GBM_FORMAT_ABGR8888, GBM_BO_USE_LINEAR);
    assert(gbo != nullptr);
    int dmabuf_fd = gbm_bo_get_fd(gbo);
    assert(dmabuf_fd >= 0);
    int pitch = gbm_bo_get_stride(gbo);
    loadImageToGBMBUF(gbo, data, width, height);
    int format = GBM_FORMAT_XBGR8888;

    PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
    assert(eglCreateImageKHR != nullptr);
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
    assert(glEGLImageTargetTexture2DOES != nullptr);
    PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
    assert(eglDestroyImageKHR != nullptr);

    const EGLint khr_image_attrs[] = {
        EGL_DMA_BUF_PLANE0_FD_EXT, dmabuf_fd,
        EGL_WIDTH, width,
        EGL_HEIGHT, height,
        EGL_LINUX_DRM_FOURCC_EXT, format,
        EGL_DMA_BUF_PLANE0_PITCH_EXT, pitch,
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
        EGL_NONE, EGL_NONE
    };

    EGLImage eglImage = eglCreateImageKHR(eglDisplay, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, nullptr /* no client buffer */, khr_image_attrs);
    assert(eglImage != EGL_NO_IMAGE);
#endif

    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#if USE_DMABUF
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, eglImage);
    checkGlError("glEGLImageTargetTexture2DOES");
#else
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    checkGlError("glTexImage2D");
#endif
    freeImageData(data);
    shader.setInt("tex", 0);

    while(userInterrupt() == GL_FALSE)
    {
        glClearColor(0.00f, 0.70f, 0.67f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        eglSwapBuffers(eglDisplay, eglSurface);
    }

    glDeleteTextures(1, &tex_id);
#if USE_DMABUF
    eglDestroyImageKHR(eglDisplay, eglImage);
    gbm_bo_destroy(gbo);
    gbm_device_destroy(gbm);
    close(drm_fd);
#endif
    glDeleteBuffers(1, &mVBO);
    eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglTerminate(eglDisplay);

    return 0;
}
