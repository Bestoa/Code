#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GLES3/gl32.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <vector>
#include <memory>
#include <linux/input.h>

const unsigned int WINDOW_WIDTH = 800;
const unsigned int WINDOW_HEIGHT = 800;

const unsigned int VertexArray = 0;
const unsigned int TexCoordArray = 1;
GLubyte *readbuf = NULL;

bool createEGLDisplay(EGLDisplay& eglDisplay)
{
    eglDisplay = eglGetDisplay((EGLNativeDisplayType)EGL_DEFAULT_DISPLAY);
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

    // Bind the correct API
    int result = EGL_FALSE;
    result = eglBindAPI(EGL_OPENGL_ES_API);

    if (result != EGL_TRUE) { return false; }

    return true;
}

bool chooseEGLConfig(EGLDisplay eglDisplay, EGLConfig& eglConfig)
{
    const EGLint configurationAttributes[] = { EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT|EGL_OPENGL_ES3_BIT, EGL_NONE };

    EGLint configsReturned;
    if (!eglChooseConfig(eglDisplay, configurationAttributes, &eglConfig, 1, &configsReturned) || (configsReturned != 1))
    {
        printf("Failed to choose a suitable config.");
        return false;
    }
    return true;
}

bool createEGLSurface(EGLDisplay eglDisplay, EGLConfig eglConfig, EGLSurface& eglSurface)
{
    EGLint pbuf[] = {EGL_WIDTH, WINDOW_WIDTH, EGL_HEIGHT, WINDOW_HEIGHT, EGL_NONE};
    eglSurface = eglCreatePbufferSurface(eglDisplay, eglConfig, pbuf);
    if (readbuf == NULL) {
        printf("Malloc pbuffer\n");
        readbuf = (GLubyte *)malloc(WINDOW_WIDTH * WINDOW_HEIGHT * 4);
    }
    return true;
}

bool setupEGLContext(EGLDisplay eglDisplay, EGLConfig eglConfig, EGLSurface eglSurface, EGLContext& context)
{
    eglBindAPI(EGL_OPENGL_ES_API);

    EGLint contextAttributes[] = { EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 2, EGL_NONE };

    context = eglCreateContext(eglDisplay, eglConfig, NULL, contextAttributes);
    eglMakeCurrent(eglDisplay, eglSurface, eglSurface, context);

    return true;
}

bool loadTexture(const char *name)
{
    //stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char *data = stbi_load(name, &width, &height, &nrChannels, 0);

    if (data == nullptr)
    {
        printf("Load texture Failed.\n");
        return false;
    }

    unsigned int texture;
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

    return true;
error:
    glDeleteTextures(1, &texture);
    return false;
}


bool initializeBuffer(GLuint& vertexBuffer)
{
    // x, y, z, u, v
    GLfloat vertexData[] = {
        -0.9f, 0.9f, 0.0f,  0.0f, 1.0f,
        -0.9f, -0.9f, 0.0f, 0.0f, 0.0f,
        0.9f,  -0.9f,  0.0f, 1.0f, 0.0f,
        -0.9f, 0.9f, 0.0f, 0.0f, 1.0f,
        0.9f,  -0.9f,  0.0f, 1.0f, 0.0f,
        0.9f, 0.9f, 0.0f, 1.0f, 1.0f,
    };

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

    return true;
}

bool initializeShaders(GLuint& shaderProgram)
{
    GLuint fragmentShader = 0;
    GLuint vertexShader = 0;

    const char* const fragmentShaderSource = "\
        #version 320 es\n \
        in highp vec2 texCoord; \
        out highp vec4 color; \
        uniform sampler2D tex; \
        void main (void)\
        {\
            color = texture(tex, texCoord); \
        }";

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragmentShader, 1, (const char**)&fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    GLint isShaderCompiled;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isShaderCompiled);
    if (!isShaderCompiled)
    {
        int infoLogLength, charactersWritten;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<char> infoLog;
        infoLog.resize(infoLogLength);
        glGetShaderInfoLog(fragmentShader, infoLogLength, &charactersWritten, infoLog.data());

        infoLogLength > 1 ? printf("%s\n", infoLog.data()) : printf("Failed to compile fragment shader.\n");

        return false;
    }

    const char* const vertexShaderSource = "\
        #version 320 es\n \
        in highp vec3 myVertex;\
        in highp vec2 myTexCoord; \
        out highp vec2 texCoord; \
        uniform mediump mat4 transformationMatrix;\
        void main(void)\
        {\
            gl_Position = transformationMatrix * vec4(myVertex, 1.0f);\
            texCoord = myTexCoord; \
        }";

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, (const char**)&vertexShaderSource, NULL);

    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isShaderCompiled);
    if (!isShaderCompiled)
    {
        int infoLogLength, charactersWritten;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<char> infoLog;
        infoLog.resize(infoLogLength);
        glGetShaderInfoLog(vertexShader, infoLogLength, &charactersWritten, infoLog.data());

        infoLogLength > 1 ? printf("%s\n", infoLog.data()) : printf("Failed to compile vertex shader.\n");
        return false;
    }

    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, fragmentShader);
    glAttachShader(shaderProgram, vertexShader);

    glBindAttribLocation(shaderProgram, VertexArray, "myVertex");
    glBindAttribLocation(shaderProgram, TexCoordArray, "myTexCoord");

    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLint isLinked;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &isLinked);
    if (!isLinked)
    {
        int infoLogLength, charactersWritten;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<char> infoLog;
        infoLog.resize(infoLogLength);
        glGetShaderInfoLog(shaderProgram, infoLogLength, &charactersWritten, infoLog.data());

        infoLogLength > 1 ? printf("%s\n", infoLog.data()) : printf("Failed to link shader program.\n");
        return false;
    }

    glUseProgram(shaderProgram);
    return true;
}

bool renderScene(GLuint shaderProgram, GLuint vertexBuffer, EGLDisplay eglDisplay, EGLSurface eglSurface)
{
    static int frame = 0;
    glClearColor(0.00f, 0.70f, 0.67f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

    int matrixLocation = glGetUniformLocation(shaderProgram, "transformationMatrix");

    glm::mat4 transformationMatrix = glm::mat4(1.0f);
    glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, &transformationMatrix[0][0]);

    glEnableVertexAttribArray(VertexArray);
    glVertexAttribPointer(VertexArray, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);

    glEnableVertexAttribArray(TexCoordArray);
    glVertexAttribPointer(TexCoordArray, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    int texLocation = glGetUniformLocation(shaderProgram, "tex");
    glUniform1i(texLocation, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glReadPixels(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, readbuf);
    char name[20] = { 0 };
    sprintf(name, "%d.png", frame);
    stbi_write_png(name, WINDOW_WIDTH, WINDOW_HEIGHT, 4, readbuf, 0);
    frame++;

    return true;
}

void deInitializeGLState(GLuint shaderProgram, GLuint vertexBuffer)
{
    glDeleteProgram(shaderProgram);
    glDeleteBuffers(1, &vertexBuffer);
}

void releaseEGLState(EGLDisplay eglDisplay)
{
    if (eglDisplay != NULL)
    {
        eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglTerminate(eglDisplay);
    }
    if (readbuf)
        free(readbuf);
}

bool render(GLuint shaderProgram, GLuint vertexBuffer, EGLDisplay& eglDisplay, EGLSurface& eglSurface)
{
    int end = 1;
    for (int i = 0; i < end; ++i)
    {
        if (!renderScene(shaderProgram, vertexBuffer, eglDisplay, eglSurface)) { return false; }
    }
    return true;
}


int main(int /*argc*/, char** /*argv*/)
{
    EGLDisplay eglDisplay = NULL;
    EGLConfig eglConfig = NULL;
    EGLSurface eglSurface = NULL;
    EGLContext context = NULL;

    GLuint shaderProgram = 0;

    GLuint vertexBuffer = 0;

    createEGLDisplay(eglDisplay) &&
        chooseEGLConfig(eglDisplay, eglConfig) &&
        createEGLSurface(eglDisplay, eglConfig, eglSurface) &&
        setupEGLContext(eglDisplay, eglConfig, eglSurface, context) &&
        initializeBuffer(vertexBuffer) &&
        initializeShaders(shaderProgram) &&
        loadTexture("./tex.tga") &&
        render(shaderProgram, vertexBuffer, eglDisplay, eglSurface);

    deInitializeGLState(shaderProgram, vertexBuffer);
    releaseEGLState(eglDisplay);
    return 0;
}
