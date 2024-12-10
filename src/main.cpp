#include <SDL.h>
#include <GL/glew.h>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "shaders.h"
#include "helpers.h"
#include "gradient_cpu.h"

void printLog(GLuint resource);
GLuint makeShader(GLint type, const char* src);
GLuint makeBuffer(GLenum type, GLenum usage, void* data, size_t len);

int main(int argc, char** argv) 
{
    // Initialize SDL and create a window
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return -1;
    SDL_Window* window = SDL_CreateWindow("Gradient Shader", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(window);

    glewInit();

    // Check if we want to use GPU or CPU
    bool useGPU = false;

    if (useGPU) {
        // Setup OpenGL shaders
        GLuint vertexShader = makeShader(GL_VERTEX_SHADER, vShaderSource);
        GLuint fragmentShader = makeShader(GL_FRAGMENT_SHADER, fShaderSource);
        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        GLfloat quad[] = {
            -1.0f, -1.0f,
             1.0f, -1.0f,
             1.0f,  1.0f,
            -1.0f,  1.0f
        };

        GLuint quadBuffer = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, quad, sizeof(quad));

        while (true) {
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(program);
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, quadBuffer);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            glDisableVertexAttribArray(0);

            SDL_GL_SwapWindow(window);
        }
    } else {
        const int width = 800, height = 600;
        std::vector<Color> framebuffer(width * height);
        generateGradientCPU(framebuffer, width, height);
        // Display gradient here (using SDL surfaces)
    }

    SDL_Quit();
    return 0;

}