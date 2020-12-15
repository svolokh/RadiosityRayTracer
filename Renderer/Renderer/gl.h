#pragma once

#include <cassert>

#include <GL/glew.h>

#if defined(DEBUG)
#define XGL_POST() { \
	if(glGetError() != GL_NO_ERROR) { \
		assert(!"OpenGL call failed"); \
	} \
}
#else
#define XGL_POST()
#endif
#define XGL(EXPR) { EXPR; XGL_POST() }