#pragma once

#include "vec3.h"

#include <GL/glew.h>

namespace rt {

// A vertex of a patch (stored in a vertex buffer for rendering hemicube faces).
struct patch_vertex {
	vec3 pos; // World-space position of this patch
	GLuint index; // Patch index
};

}