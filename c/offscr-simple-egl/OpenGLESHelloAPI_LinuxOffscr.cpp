#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GLES3/gl32.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <png.h>

#include <vector>
#include <memory>
#include <linux/input.h>

// Width and height of the window
const unsigned int WINDOW_WIDTH = 300;
const unsigned int WINDOW_HEIGHT = 300;

// Index to bind the attributes to vertex shaders
const unsigned int VertexArray = 0;
GLubyte *readbuf = NULL;

/*!*********************************************************************************************************************
  \param[in]			functionLastCalled          Function which triggered the error
  \return		True if no EGL error was detected
  \brief	Tests for an EGL error and prints it.
 ***********************************************************************************************************************/
bool testEGLError(const char* functionLastCalled)
{
    //	eglGetError returns the last error that occurred using EGL, not necessarily the status of the last called function. The user has to
    //	check after every single EGL call or at least once every frame. Usually this would be for debugging only, but for this example
    //	it is enabled always.
    EGLint lastError = eglGetError();
    if (lastError != EGL_SUCCESS)
    {
        printf("%s failed (%x).\n", functionLastCalled, lastError);
        return false;
    }
    return true;
}

/*!*********************************************************************************************************************
  \param[in]			functionLastCalled          Function which triggered the error
  \return		True if no GL error was detected
  \brief	Tests for an GL error and prints it in a message box.
 ***********************************************************************************************************************/
bool testGLError(const char* functionLastCalled)
{
    //	glGetError returns the last error that occurred using OpenGL ES, not necessarily the status of the last called function. The user
    //	has to check after every single OpenGL ES call or at least once every frame. Usually this would be for debugging only, but for this
    //	example it is enabled always
    GLenum lastError = glGetError();
    if (lastError != GL_NO_ERROR)
    {
        printf("%s failed (%x).\n", functionLastCalled, lastError);
        return false;
    }
    return true;
}

/*!*********************************************************************************************************************
  \param[out]		eglDisplay				    EGLDisplay created from nativeDisplay
  \return		Whether the function succeeded or not.
  \brief	Creates an EGLDisplay from a native native display, and initializes it.
 ***********************************************************************************************************************/
bool createEGLDisplay(EGLDisplay& eglDisplay)
{
    //	Get an EGL display.
    //	EGL uses the concept of a "display" which in most environments corresponds to a single physical screen. After creating a native
    //	display for a given windowing system, EGL can use this handle to get a corresponding EGLDisplay handle to it for use in rendering.
    //	Should this fail, EGL is usually able to provide access to a default display.
    eglDisplay = eglGetDisplay((EGLNativeDisplayType)EGL_DEFAULT_DISPLAY);
    // If a display couldn't be obtained, return an error.
    if (eglDisplay == EGL_NO_DISPLAY)
    {
        printf("Failed to get an EGLDisplay\n");
        return false;
    }

    //	Initialize EGL.
    //	EGL has to be initialized with the display obtained in the previous step. All EGL functions other than eglGetDisplay
    //	and eglGetError need an initialized EGLDisplay.
    //	If an application is not interested in the EGL version number it can just pass NULL for the second and third parameters, but they
    //	are queried here for illustration purposes.
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

/*!*********************************************************************************************************************
  \param[in]			eglDisplay                  The EGLDisplay used by the application
  \param[out]		eglConfig                   The EGLConfig chosen by the function
  \return		Whether the function succeeded or not.
  \brief	Chooses an appropriate EGLConfig and return it.
 ***********************************************************************************************************************/
bool chooseEGLConfig(EGLDisplay eglDisplay, EGLConfig& eglConfig)
{
    //	Specify the required configuration attributes.
    //	An EGL "configuration" describes the capabilities an application requires and the type of surfaces that can be used for drawing.
    //	Each implementation exposes a number of different configurations, and an application needs to describe to EGL what capabilities it
    //	requires so that an appropriate one can be chosen. The first step in doing this is to create an attribute list, which is an array
    //	of key/value pairs which describe particular capabilities requested. In this application nothing special is required so we can query
    //	the minimum of needing it to render to a window, and being OpenGL ES 3.0 capable.
    const EGLint configurationAttributes[] = { EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT|EGL_OPENGL_ES3_BIT, EGL_NONE };

    //	Find a suitable EGLConfig
    //	eglChooseConfig is provided by EGL to provide an easy way to select an appropriate configuration. It takes in the capabilities
    //	specified in the attribute list, and returns a list of available configurations that match or exceed the capabilities requested.
    //	Details of all the possible attributes and how they are selected for by this function are available in the EGL reference pages here:
    //	http://www.khronos.org/registry/egl/sdk/docs/man/xhtml/eglChooseConfig.html
    //	It is also possible to simply get the entire list of configurations and use a custom algorithm to choose a suitable one, as many
    //	advanced applications choose to do. For this application however, taking the first EGLConfig that the function returns suits
    //	its needs perfectly, so we limit it to returning a single EGLConfig.
    EGLint configsReturned;
    if (!eglChooseConfig(eglDisplay, configurationAttributes, &eglConfig, 1, &configsReturned) || (configsReturned != 1))
    {
        printf("Failed to choose a suitable config.");
        return false;
    }
    return true;
}

/*!*********************************************************************************************************************
  \param[in]			eglDisplay                  The EGLDisplay used by the application
  \param[in]			eglConfig                   An EGLConfig chosen by the application
  \param[out]		eglSurface					The EGLSurface created from the native window.
  \return		Whether the function succeeds or not.
  \brief	Creates an EGLSurface from a native window
 ***********************************************************************************************************************/
bool createEGLSurface(EGLDisplay eglDisplay, EGLConfig eglConfig, EGLSurface& eglSurface)
{
    //	Create an EGLSurface for rendering.
    //	Using a native window created earlier and a suitable eglConfig, a surface is created that can be used to render OpenGL ES calls to.
    //	There are three main surface types in EGL, which can all be used in the same way once created but work slightly differently:
    //	 - Window Surfaces  - These are created from a native window and are drawn to the screen.
    //	 - Pixmap Surfaces  - These are created from a native windowing system as well, but are offscreen and are not displayed to the user.
    //	 - PBuffer Surfaces - These are created directly within EGL, and like Pixmap Surfaces are offscreen and thus not displayed.
    //	The offscreen surfaces are useful for non-rendering contexts and in certain other scenarios, but for most applications the main
    //	surface used will be a window surface as performed below.
    EGLint pbuf[] = {EGL_WIDTH, WINDOW_WIDTH, EGL_HEIGHT, WINDOW_HEIGHT, EGL_NONE};
    eglSurface = eglCreatePbufferSurface(eglDisplay, eglConfig, pbuf);
    if (!testEGLError("eglCreatePbufferSurface")) { return false; }
    if (readbuf == NULL) {
        printf("Malloc pbuffer\n");
        readbuf = (GLubyte *)malloc(WINDOW_WIDTH * WINDOW_HEIGHT * 4);
    }
    return true;
}

/*!*********************************************************************************************************************
  \param[in]			eglDisplay                  The EGLDisplay used by the application
  \param[in]			eglConfig                   An EGLConfig chosen by the application
  \param[in]			eglSurface					The EGLSurface created from the native window.
  \param[out]		context                  The EGLContext created by this function
  \param[in]			nativeWindow                A native window, used to display error messages
  \return		Whether the function succeeds or not.
  \brief	Sets up the EGLContext, creating it and then installing it to the current thread.
 ***********************************************************************************************************************/
bool setupEGLContext(EGLDisplay eglDisplay, EGLConfig eglConfig, EGLSurface eglSurface, EGLContext& context)
{
    //	Make OpenGL ES the current API.
    // EGL needs a way to know that any subsequent EGL calls are going to be affecting OpenGL ES,
    // rather than any other API (such as OpenVG).
    eglBindAPI(EGL_OPENGL_ES_API);
    if (!testEGLError("eglBindAPI")) { return false; }

    //	Create a context.
    //	EGL has to create what is known as a context for OpenGL ES. The concept of a context is OpenGL ES's way of encapsulating any
    //	resources and state. What appear to be "global" functions in OpenGL actually only operate on the current context. A context
    //	is required for any operations in OpenGL ES.
    //	Similar to an EGLConfig, a context takes in a list of attributes specifying some of its capabilities. However in most cases this
    //	is limited to just requiring the version of the OpenGL ES context required - In this case, OpenGL ES 3.2.
    EGLint contextAttributes[] = { EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 2, EGL_NONE };

    // Create the context with the context attributes supplied
    context = eglCreateContext(eglDisplay, eglConfig, NULL, contextAttributes);
    if (!testEGLError("eglCreateContext")) { return false; }

    //	Bind the context to the current thread.
    //	Due to the way OpenGL uses global functions, contexts need to be made current so that any function call can operate on the correct
    //	context. Specifically, make current will bind the context to the current rendering thread it's called from. If the calling thread already
    //  has a current rendering context then that context is flushed and marked as no longer current. It is not valid to call eglMakeCurrent with a context
    //  which is current on another thread.
    //  To use multiple contexts at the same time, users should use multiple threads and synchronise between them.eglMakeCurrent(eglDisplay, eglSurface, eglSurface, context);
    eglMakeCurrent(eglDisplay, eglSurface, eglSurface, context);

    if (!testEGLError("eglMakeCurrent")) { return false; }
    return true;
}

/*!*********************************************************************************************************************
  \param[out]		vertexBuffer     Handle to a vertex buffer object
  \return		Whether the function succeeds or not.
  \brief	Initializes shaders, buffers and other state required to begin rendering with OpenGL ES
 ***********************************************************************************************************************/
bool initializeBuffer(GLuint& vertexBuffer)
{
    //	Concept: Vertices
    //	When rendering a polygon or model to screen, OpenGL ES has to be told where to draw the object, and more fundamentally what shape
    //	it is. The data used to do this is referred to as vertices, points in 3D space which are usually collected into groups of three
    //	to render as triangles. Fundamentally, any advanced 3D shape in OpenGL ES is constructed from a series of these vertices - each
    //	vertex representing one corner of a polygon.

    //	Concept: Buffer Objects
    //	To operate on any data, OpenGL first needs to be able to access it. The GPU maintains a separate pool of memory it uses independent
    //	of the CPU. Whilst on many embedded systems these are in the same physical memory, the distinction exists so that they can use and
    //	allocate memory without having to worry about synchronising with any other processors in the device.
    //	To this end, data needs to be uploaded into buffers, which are essentially a reserved bit of memory for the GPU to use. By creating
    //	a buffer and giving it some data we can tell the GPU how to render a triangle.

    // Vertex data containing the positions of each point of the triangle
    GLfloat vertexData[] = {
        -0.574f, -0.33f, 0.0f,  // Bottom Left
        0.574f, -0.33f, 0.0f,  // Bottom Right
        0.0f,   0.66f,  0.0f}; // Top Middle

    // Generate a buffer object
    glGenBuffers(1, &vertexBuffer);

    // Bind buffer as an vertex buffer so we can fill it with data
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

    //	Set the buffer's size, data and usage
    //	Note the last argument - GL_STATIC_DRAW. This tells the driver that we intend to read from the buffer on the GPU, and don't intend
    //	to modify the data until we're done with it.
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

    if (!testGLError("glBufferData")) { return false; }
    return true;
}

/*!*********************************************************************************************************************
  \param[out]		shaderProgram               Handle to a shader program containing the fragment and vertex shader
  \return		Whether the function succeeds or not.
  \brief	Initializes shaders, buffers and other state required to begin rendering with OpenGL ES
 ***********************************************************************************************************************/
bool initializeShaders(GLuint& shaderProgram)
{
    //	Concept: Shaders
    //	OpenGL ES 2.0 uses what are known as shaders to determine how to draw objects on the screen. Instead of the fixed function
    //	pipeline in early OpenGL or OpenGL ES 1.x, users can now programmatically define how vertices are transformed on screen, what
    //	data is used where, and how each pixel on the screen is colored.
    //	These shaders are written in GL Shading Language ES: http://www.khronos.org/registry/gles/specs/2.0/GLSL_ES_Specification_1.0.17.pdf
    //	which is usually abbreviated to simply "GLSL ES".
    //	Each shader is compiled on-device and then linked into a shader program, which combines a vertex and fragment shader into a form
    //	that the OpenGL ES implementation can execute.

    //	Concept: Fragment Shaders
    //	In a final buffer of image data, each individual point is referred to as a pixel. Fragment shaders are the part of the pipeline
    //	which determine how these final pixels are colored when drawn to the framebuffer. When data is passed through here, the positions
    //	of these pixels is already set, all that's left to do is set the final color based on any defined inputs.
    //	The reason these are called "fragment" shaders instead of "pixel" shaders is due to a small technical difference between the two
    //	concepts. When you color a fragment, it may not be the final color which ends up on screen. This is particularly true when
    //	performing blending, where multiple fragments can contribute to the final pixel color.

    GLuint fragmentShader = 0;
    GLuint vertexShader = 0;

    const char* const fragmentShaderSource = "\
        #version 320 es\n \
        out highp vec4 color; \
        void main (void)\
        {\
            color = vec4(1.0, 1.0, 0.0, 1.0);\
        }";

    // Create a fragment shader object
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // Load the source code into it
    glShaderSource(fragmentShader, 1, (const char**)&fragmentShaderSource, NULL);

    // Compile the source code
    glCompileShader(fragmentShader);

    // Check that the shader compiled
    GLint isShaderCompiled;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isShaderCompiled);
    if (!isShaderCompiled)
    {
        // If an error happened, first retrieve the length of the log message
        int infoLogLength, charactersWritten;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLogLength);

        // Allocate enough space for the message and retrieve it
        std::vector<char> infoLog;
        infoLog.resize(infoLogLength);
        glGetShaderInfoLog(fragmentShader, infoLogLength, &charactersWritten, infoLog.data());

        // Display the error in a dialog box
        infoLogLength > 1 ? printf("%s\n", infoLog.data()) : printf("Failed to compile fragment shader.\n");

        return false;
    }

    //	Concept: Vertex Shaders
    //	Vertex shaders primarily exist to allow a developer to express how to orient vertices in 3D space, through transformations like
    //	Scaling, Translation or Rotation. Using the same basic layout and structure as a fragment shader, these take in vertex data and
    //	output a fully transformed set of positions. Other inputs are also able to be used such as normals or texture coordinates, and can
    //	also be transformed and output alongside the position data.

    // Vertex shader code
    const char* const vertexShaderSource = "\
        #version 320 es\n \
        in highp vec3 myVertex;\
        uniform mediump mat4 transformationMatrix;\
        void main(void)\
        {\
            gl_Position = transformationMatrix * vec4(myVertex, 1.0f);\
        }";

    // Create a vertex shader object
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    // Load the source code into the shader
    glShaderSource(vertexShader, 1, (const char**)&vertexShaderSource, NULL);

    // Compile the shader
    glCompileShader(vertexShader);

    // Check the shader has compiled
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isShaderCompiled);
    if (!isShaderCompiled)
    {
        // If an error happened, first retrieve the length of the log message
        int infoLogLength, charactersWritten;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLogLength);

        // Allocate enough space for the message and retrieve it
        std::vector<char> infoLog;
        infoLog.resize(infoLogLength);
        glGetShaderInfoLog(vertexShader, infoLogLength, &charactersWritten, infoLog.data());

        // Display the error in a dialog box
        infoLogLength > 1 ? printf("%s\n", infoLog.data()) : printf("Failed to compile vertex shader.\n");
        return false;
    }

    // Create the shader program
    shaderProgram = glCreateProgram();

    // Attach the fragment and vertex shaders to it
    glAttachShader(shaderProgram, fragmentShader);
    glAttachShader(shaderProgram, vertexShader);

    // Bind the vertex attribute "myVertex" to location VertexArray (0)
    glBindAttribLocation(shaderProgram, VertexArray, "myVertex");

    // Link the program
    glLinkProgram(shaderProgram);

    // After linking the program, shaders are no longer necessary
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Check if linking succeeded in the same way we checked for compilation success
    GLint isLinked;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &isLinked);
    if (!isLinked)
    {
        // If an error happened, first retrieve the length of the log message
        int infoLogLength, charactersWritten;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &infoLogLength);

        // Allocate enough space for the message and retrieve it
        std::vector<char> infoLog;
        infoLog.resize(infoLogLength);
        glGetShaderInfoLog(shaderProgram, infoLogLength, &charactersWritten, infoLog.data());

        // Display the error in a dialog box
        infoLogLength > 1 ? printf("%s\n", infoLog.data()) : printf("Failed to link shader program.\n");
        return false;
    }

    //	Use the Program
    //	Calling glUseProgram tells OpenGL ES that the application intends to use this program for rendering. Now that it's installed into
    //	the current state, any further glDraw* calls will use the shaders contained within it to process scene data. Only one program can
    //	be active at once, so in a multi-program application this function would be called in the render loop. Since this application only
    //	uses one program it can be installed in the current state and left there.
    glUseProgram(shaderProgram);
    if (!testGLError("glUseProgram")) { return false; }
    return true;
}

// Copy from https://gist.github.com/niw/5963798
void write_png_file(char *filename, void *buffer, int width, int height) {

    png_bytep row_pointers[height];

    FILE *fp = fopen(filename, "wb");
    if(!fp) abort();

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) abort();

    png_infop info = png_create_info_struct(png);
    if (!info) abort();

    if (setjmp(png_jmpbuf(png))) abort();

    png_init_io(png, fp);

    // Output is 8bit depth, RGBA format.
    png_set_IHDR(
            png,
            info,
            width, height,
            8,
            PNG_COLOR_TYPE_RGBA,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT
            );
    png_write_info(png, info);

    // To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
    // Use png_set_filler().
    //png_set_filler(png, 0, PNG_FILLER_AFTER);

    for(int i = 0; i < height; i++) {
        row_pointers[i] = (unsigned char *)(buffer + i * WINDOW_WIDTH * 4);
    }
    png_write_image(png, row_pointers);
    png_write_end(png, NULL);

    fclose(fp);

    png_destroy_write_struct(&png, &info);
}

/*!*********************************************************************************************************************
  \param[in]			shaderProgram               The shader program used to render the scene
  \param[in]			eglDisplay                  The EGLDisplay used by the application
  \param[in]			eglSurface					The EGLSurface created from the native window.
  \param[in]			nativeDisplay				The native display used by the application
  \return		Whether the function succeeds or not.
  \brief	Renders the scene to the framebuffer. Usually called within a loop.
 ***********************************************************************************************************************/
bool renderScene(GLuint shaderProgram, GLuint vertexBuffer, EGLDisplay eglDisplay, EGLSurface eglSurface)
{
    static int frame = 0;
    //	Set the clear color
    //	At the start of a frame, generally you clear the image to tell OpenGL ES that you're done with whatever was there before and want to
    //	draw a new frame. In order to do that however, OpenGL ES needs to know what color to set in the image's place. glClearColor
    //	sets this value as 4 floating point values between 0.0 and 1.0, as the Red, Green, Blue and Alpha channels. Each value represents
    //	the intensity of the particular channel, with all 0.0 being transparent black, and all 1.0 being opaque white. Subsequent calls to
    //	glClear with the color bit will clear the frame buffer to this value.
    //	The functions glClearDepth and glClearStencil allow an application to do the same with depth and stencil values respectively.
    glClearColor(0.00f, 0.70f, 0.67f, 1.0f);

    //	Clears the color buffer.
    //	glClear is used here with the Color Buffer to clear the color. It can also be used to clear the depth or stencil buffer using
    //	GL_DEPTH_BUFFER_BIT or GL_STENCIL_BUFFER_BIT, respectively.
    glClear(GL_COLOR_BUFFER_BIT);

    //	Bind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

    // Get the location of the transformation matrix in the shader using its name
    int matrixLocation = glGetUniformLocation(shaderProgram, "transformationMatrix");

    // Matrix used to specify the orientation of the triangle on screen.
    // const float transformationMatrix[] = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
    glm::mat4 transformationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(10.0f * frame), glm::vec3(0.0f, 0.0f, 1.0f));

    // Pass the transformationMatrix to the shader using its location
    glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, &transformationMatrix[0][0]);
    if (!testGLError("glUniformMatrix4fv")) { return false; }

    // Enable the user-defined vertex array
    glEnableVertexAttribArray(VertexArray);

    // Sets the vertex data to this attribute index, with the number of floats in each position
    glVertexAttribPointer(VertexArray, 3, GL_FLOAT, GL_FALSE, 0, 0);
    if (!testGLError("glVertexAttribPointer")) { return false; }

    //	Draw the triangle
    //	glDrawArrays is a draw call, and executes the shader program using the vertices and other state set by the user. Draw calls are the
    //	functions which tell OpenGL ES when to actually draw something to the framebuffer given the current state.
    //	glDrawArrays causes the vertices to be submitted sequentially from the position given by the "first" argument until it has processed
    //	"count" vertices. Other draw calls exist, notably glDrawElements which also accepts index data to allow the user to specify that
    //	some vertices are accessed multiple times, without copying the vertex multiple times.
    //	Others include versions of the above that allow the user to draw the same object multiple times with slightly different data, and
    //	a version of glDrawElements which allows a user to restrict the actual indices accessed.
    glDrawArrays(GL_TRIANGLES, 0, 3);
    if (!testGLError("glDrawArrays")) { return false; }

    //	Present the display data to the screen.
    //	When rendering to a Window surface, OpenGL ES is double buffered. This means that OpenGL ES renders directly to one frame buffer,
    //	known as the back buffer, whilst the display reads from another - the front buffer. eglSwapBuffers signals to the windowing system
    //	that OpenGL ES 2.0 has finished rendering a scene, and that the display should now draw to the screen from the new data. At the same
    //	time, the front buffer is made available for OpenGL ES 2.0 to start rendering to. In effect, this call swaps the front and back
    //	buffers.
    glReadPixels(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, readbuf);
    char name[20] = { 0 };
    sprintf(name, "%d.png", frame);
    write_png_file(name, readbuf, WINDOW_WIDTH, WINDOW_HEIGHT);
    frame++;

    return true;
}

/*!*********************************************************************************************************************
  \param[in]			shaderProgram               Handle to a shader program containing the fragment and vertex shader
  \param[in]			vertexBuffer                Handle to a vertex buffer object
  \brief	Releases the resources created by "InitializeGLState"
 ***********************************************************************************************************************/
void deInitializeGLState(GLuint shaderProgram, GLuint vertexBuffer)
{
    // Frees the OpenGL handles for the program and the VBO
    glDeleteProgram(shaderProgram);
    glDeleteBuffers(1, &vertexBuffer);
}

/*!*********************************************************************************************************************
  \param[in]			eglDisplay                   The EGLDisplay used by the application
  \brief	Releases all resources allocated by EGL
 ***********************************************************************************************************************/
void releaseEGLState(EGLDisplay eglDisplay)
{
    if (eglDisplay != NULL)
    {
        // To release the resources in the context, first the context has to be released from its binding with the current thread.
        eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        // Terminate the display, and any resources associated with it (including the EGLContext)
        eglTerminate(eglDisplay);
    }
    if (readbuf)
        free(readbuf);
}

bool render(GLuint shaderProgram, GLuint vertexBuffer, EGLDisplay& eglDisplay, EGLSurface& eglSurface)
{
    // Renders a triangle for 10 frames using the state setup in the previous function
    for (int i = 0; i < 10; ++i)
    {
        if (!renderScene(shaderProgram, vertexBuffer, eglDisplay, eglSurface)) { return false; }
    }
    return true;
}

/*!*********************************************************************************************************************
  \param[in]			argc           Number of arguments passed to the application, ignored.
  \param[in]			argv           Command line strings passed to the application, ignored.
  \return		Result code to send to the Operating System
  \brief	Main function of the program, executes other functions.
 ***********************************************************************************************************************/
int main(int /*argc*/, char** /*argv*/)
{
    // EGL variables
    EGLDisplay eglDisplay = NULL;
    EGLConfig eglConfig = NULL;
    EGLSurface eglSurface = NULL;
    EGLContext context = NULL;

    // Handles for the program, used to draw the triangle, and the program handle which combines them.
    GLuint shaderProgram = 0;

    // A vertex buffer object to store our model data.
    GLuint vertexBuffer = 0;

    // Perform the chain of initialisation step (stop if anything fails)

    // Create and Initialize an EGLDisplay from the native display and ...
    createEGLDisplay(eglDisplay) &&

        // Choose an EGLConfig for the application, used when setting up the rendering surface and EGLContext
        chooseEGLConfig(eglDisplay, eglConfig) &&

        // Create an EGLSurface for rendering from the native window
        createEGLSurface(eglDisplay, eglConfig, eglSurface) &&

        // Setup the EGL Context from the other EGL constructs created so far, so that the application is ready to submit OpenGL ES commands
        setupEGLContext(eglDisplay, eglConfig, eglSurface, context) &&

        // Initialize the vertex data in the application
        initializeBuffer(vertexBuffer) &&

        // Initialize the fragment and vertex shaders used in the application
        initializeShaders(shaderProgram) &&

        // If everything else succeeded, run the rendering loop.
        render(shaderProgram, vertexBuffer, eglDisplay, eglSurface);

    // Cleanup
    deInitializeGLState(shaderProgram, vertexBuffer);
    // Release the EGL State
    releaseEGLState(eglDisplay);
    return 0;
}
