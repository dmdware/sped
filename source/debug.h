


#ifndef DEBUG_H
#define DEBUG_H

#include "platform.h"

void LastNum(const char* l);
void LastNum(const char* f, const int line);
#ifdef GLDEBUG
void CheckGLError(const char* file, int line);
#endif
GLvoid APIENTRY GLMessageHandler(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam);

#endif
