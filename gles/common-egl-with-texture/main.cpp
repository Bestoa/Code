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

#define USE_DMABUF 0

#if USE_DMABUF
#include <gbm.h>
#include <xf86drm.h>
#include <drm_fourcc.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "shader.h"

using namespace std;
using namespace glm;

#define WIDTH (600)
#define HEIGHT (600)

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

extern bool nativeInit(int w, int h, EGLNativeDisplayType *eglNativeDisplay, EGLNativeWindowType *eglNativeWindow);
extern void nativeDestroy();
extern void pollEvent();
extern bool shouldStop();

int main()
{
    int tex_w, tex_h;
    uint8_t *data;
    assert(loadImageData("tex.tga", &data, &tex_w, &tex_h) == true);
    printf("Use GBM_TEX tex = %d\n", USE_DMABUF);
#if USE_DMABUF
    // hard code card1 here for my amd card
    int drm_fd = open("/dev/dri/card1", O_RDWR | O_CLOEXEC);
    assert(drm_fd >= 0);
    struct gbm_device* gbm = gbm_create_device(drm_fd);
    assert(gbm != nullptr);
    struct gbm_bo *gbo = gbm_bo_create(gbm, tex_w, tex_h, GBM_FORMAT_ABGR8888, GBM_BO_USE_LINEAR);
    assert(gbo != nullptr);
    int dmabuf_fd = gbm_bo_get_fd(gbo);
    assert(dmabuf_fd >= 0);
    int pitch = gbm_bo_get_stride(gbo);
    loadImageToGBMBUF(gbo, data, tex_w, tex_h);
    int format = GBM_FORMAT_XBGR8888;

    PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
    assert(eglCreateImageKHR != nullptr);
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
    assert(glEGLImageTargetTexture2DOES != nullptr);
    PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
    assert(eglDestroyImageKHR != nullptr);
#endif

    // EGL start
    EGLNativeDisplayType eglNativeDisplay;
    EGLNativeWindowType eglNativeWindow;
    assert(nativeInit(WIDTH, HEIGHT, &eglNativeDisplay, &eglNativeWindow) == EGL_TRUE);

    EGLDisplay eglDisplay = eglGetDisplay(eglNativeDisplay);
    assert(eglDisplay != EGL_NO_DISPLAY);

    EGLint eglMajorVersion = 0;
    EGLint eglMinorVersion = 0;
    assert(eglInitialize(eglDisplay, &eglMajorVersion, &eglMinorVersion));

    printf("EGL version: %d.%d\n", eglMajorVersion, eglMinorVersion);

    assert(eglBindAPI(EGL_OPENGL_ES_API) == EGL_TRUE);

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
        assert(false);
    }
    EGLSurface eglSurface;
    eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, (EGLNativeWindowType)eglNativeWindow, NULL);
    assert(testEGLError("eglCreateWindowSurface"));

    EGLint contextAttributes[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 2,
        EGL_NONE
    };

    EGLContext context;
    context = eglCreateContext(eglDisplay, eglConfig, NULL, contextAttributes);
    assert(testEGLError("eglCreateContext"));

    eglMakeCurrent(eglDisplay, eglSurface, eglSurface, context);
    assert(testEGLError("eglMakeCurrent"));

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

    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#if USE_DMABUF
    const EGLint khr_image_attrs[] = {
        EGL_DMA_BUF_PLANE0_FD_EXT, dmabuf_fd,
        EGL_WIDTH, tex_w,
        EGL_HEIGHT, tex_h,
        EGL_LINUX_DRM_FOURCC_EXT, format,
        EGL_DMA_BUF_PLANE0_PITCH_EXT, pitch,
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
        EGL_NONE, EGL_NONE
    };

    EGLImage eglImage = eglCreateImageKHR(eglDisplay, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, nullptr /* no client buffer */, khr_image_attrs);
    assert(eglImage != EGL_NO_IMAGE);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, eglImage);
    checkGlError("glEGLImageTargetTexture2DOES");
#else
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_w, tex_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    checkGlError("glTexImage2D");
#endif
    freeImageData(data);
    shader.setInt("tex", 0);

    glm::mat4 mat(1.0f);
    while(!shouldStop())
    {
        pollEvent();
        glClearColor(0.00f, 0.70f, 0.67f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        mat = glm::rotate(mat, glm::radians(10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        shader.setMat4("modelMat", &mat[0][0]);
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
    nativeDestroy();

    return 0;
}
