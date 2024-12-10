#ifndef HELPERS_H
#define HELPERS_H

#include <GL/glew.h>
#include <cstddef>

void printLog(GLuint resource);
GLuint makeShader(GLint type, const char* src);
GLuint makeBuffer(GLenum type, GLenum usage, void* data, size_t len);

#endif // HELPERS_H
