#pragma once

#include "vec3.h"
#include "material.h"
#include "object.h"
#include "patch_vertex.h"
#include "patch_state.h"

#include <GL/glew.h>
#include <vector>
#include <memory>

namespace rt {

// A patch in the scene for the radiosity renderer.
struct patch {
	patch(object *obj, const material &mat, const vec3 &normal);

	object *obj() const;
	const material &mat() const;
	vec3 normal() const;

	// Render the patch to an array of triangle vertices (every 3 vertices defines a triangle).
	// patch_index specifies the index of this patch to be used in the patch_vertex struct.
	virtual void render(std::vector<patch_vertex> &vertices, GLuint patch_index) const = 0;

	virtual vec3 center() const = 0;
	virtual vec3 surface_dir() const = 0; // provides an arbitrary direction along the surface
	virtual float area() const = 0;

	std::unique_ptr<patch_state> new_state() const;
	
private:
	object *obj_;
	material mat_;
	vec3 normal_;
};

}