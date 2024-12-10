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
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return -1;
    SDL_Window* window = SDL_CreateWindow("Gradient Shader", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(window);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    glewInit();

    // OpenGL or SDL
    bool useOpenGL = true;

    if (useOpenGL) 
    {
        // OpenGL shaders
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
    } 
    else 
    {
        const int width = 800, height = 600;
        std::vector<Color> framebuffer(width * height);
        generateGradientCPU(framebuffer, width, height);
        
        SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
        if (!texture) {
            std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return -1;
        }

        // Main event loop
        bool running = true;
        SDL_Event event;

        while (running) {
            // Poll events
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false; // Exit the loop
                }
            }

            // Update the texture with the framebuffer data
            void* pixels = nullptr;
            int pitch = 0;
            if (SDL_LockTexture(texture, nullptr, &pixels, &pitch) == 0) {
                memcpy(pixels, framebuffer.data(), framebuffer.size() * sizeof(Color));
                SDL_UnlockTexture(texture);
            } else {
                std::cerr << "Failed to lock texture: " << SDL_GetError() << std::endl;
            }

            // Render the texture
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);
        }

        // Cleanup CPU resources
        SDL_DestroyTexture(texture);
    }

    SDL_Quit();
    return 0;

}