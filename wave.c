#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>

static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

GLuint LoadShader ( GLenum type, const char *shaderSrc )
{
    GLuint shader;
    GLint compiled;

    // Create the shader object
    shader = glCreateShader ( type );

    if ( shader == 0 )
    return 0;

    // Load the shader source
    glShaderSource ( shader, 1, &shaderSrc, NULL );

    // Compile the shader
    glCompileShader ( shader );

    // Check the compile status
    glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

    if (!compiled) 
    {
        GLint infoLen = 0;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if ( infoLen > 1 )
        {
            char* infoLog = malloc(sizeof(char) * infoLen);

            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            printf( "Error compiling shader:\n%s\n", infoLog );            

            free(infoLog);
        }

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

int main(void)
{
    GLFWwindow* window;
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        exit(EXIT_FAILURE);
    window = glfwCreateWindow(800, 800, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, key_callback);

    GLbyte vShaderStr[] =
        "attribute vec3 vPosition;    \n"
        "uniform float time;          \n"
        "varying vec3 v;              \n"
        "float rand(vec2 co){ return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453); } \n"
        "void main()                  \n"
        "{                            \n"
        "  v = vPosition;             \n"
        //"  v.y = cos(v.x+v.z+time)/3.0 + cos(v.z)/6.0;\n"
        "  vec3 v2 = v; \n"
        "  v2.x = v2.x + time/10000.0; \n"
        //"  v.y = rand(v2.xz)/50.0 + cos(v.x+v.z+time)/2.0; \n"
        "  v.y = cos(v.x+v.z+time)/3.0 + cos((v.x+time/5.0)*10.0)/20.0 + sin(v.x*v.z)/10.0; \n"
        "  gl_Position = vec4(v, 1.0);\n"
        "}                            \n";

    GLbyte fShaderStr[] =
        "uniform float time;                                    \n"
        "varying vec3 v;                                        \n"
        "void main()                                            \n"
        "{                                                      \n"
        "  gl_FragColor = vec4(1.0, 1.0, 1.0, 0.25); \n"
        "}                                                      \n";

    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint programObject;
    GLint linked;
    GLint timeLoc;
    float t = 0;

    // Load the vertex/fragment shaders
    vertexShader = LoadShader(GL_VERTEX_SHADER, vShaderStr);
    fragmentShader = LoadShader(GL_FRAGMENT_SHADER, fShaderStr);

    // Create the program object
    programObject = glCreateProgram();

    if (programObject == 0)
        return 0;

    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);

    // Bind vPosition to attribute 0   
    glBindAttribLocation(programObject, 0, "vPosition");

    // Link the program
    glLinkProgram(programObject);

    // Check the link status
    glGetProgramiv(programObject, GL_LINK_STATUS, &linked);

    if (!linked) 
    {
        GLint infoLen = 0;

        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 1)
        {
            char* infoLog = malloc (sizeof(char) * infoLen );

            glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
            printf( "Error linking program:\n%s\n", infoLog );            

            free(infoLog);
        }

        glDeleteProgram ( programObject );
        return GL_FALSE;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    timeLoc = glGetUniformLocation(programObject, "time");

    GLfloat vertices[3072];
    int indices[1950];
    int rows = 16;
    int columns = 64;

    // Set up vertices
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < columns; ++c) {
            int index = r*columns + c;
            vertices[3*index + 0] = ((float) c)/32.0f - 1.0;
            vertices[3*index + 1] = 0.0f;
            vertices[3*index + 2] = ((float) r)/16.0f - 1.0;
        }
    }

    int i = 0;
    for (int r = 0; r < rows - 1; ++r) {
        indices[i++] = r * columns;
        for (int c = 0; c < columns; ++c) {
            indices[i++] = r * columns + c;
            indices[i++] = (r + 1) * columns + c;
        }
        indices[i++] = (r + 1) * columns + (columns - 1);
    }

    while (!glfwWindowShouldClose(window))
    {
        int width, height, i = 0;
        glfwGetFramebufferSize(window, &width, &height);

        // Set the viewport
        glViewport(0, 0, width, height);

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the program object
        glUseProgram(programObject);

        glDisable(GL_DEPTH_TEST);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glDepthMask(0);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
        glEnableVertexAttribArray(0);

        t += 0.01;
        glUniform1f(timeLoc, t);

        glVertexPointer( 3, GL_FLOAT, 0, vertices );
        glDrawElements( GL_TRIANGLE_STRIP, 1950, GL_UNSIGNED_INT, indices );

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
