#include <GL/glew.h>
#include <SDL.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include "helpers.h"

void printLog(GLuint resource)
{
    GLsizei len, cap;
    GLenum type = glIsProgram(resource) ? GL_PROGRAM : glIsShader(resource) ? GL_SHADER : 0;
    
    if (!type)
    {
        return;
    }

    if (type == GL_PROGRAM)
    {
        glGetProgramiv(resource, GL_INFO_LOG_LENGTH, &cap);
    }
    else
    {
        glGetShaderiv(resource, GL_INFO_LOG_LENGTH, &cap);
    }

    char* log = (char*)malloc(cap);
    if (!log)
    {
        SDL_Log("malloc(): %s", strerror(errno));
        return;
    }

    if (type == GL_PROGRAM)
    {
        glGetProgramInfoLog(resource, cap, &len, log);
    }
    else
    {
        glGetShaderInfoLog(resource, cap, &len, log);
    }

    if (len > 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", log);
    }

    free(log);
}

GLuint makeShader(GLint type, const char* src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    
    if (ok == GL_FALSE)
    {
        printLog(shader);
        return -1;
    }

    return shader;
}

GLuint makeBuffer(GLenum type, GLenum usage, void* data, size_t len)
{
    GLuint buf;
    glGenBuffers(1, &buf);
    glBindBuffer(type, buf);
    glBufferData(type, len, data, usage);
    
    return buf;
}
