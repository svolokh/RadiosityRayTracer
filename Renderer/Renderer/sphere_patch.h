#pragma once

#include "patch.h"
#include "patch_state.h"

namespace rt {

// A (curved) patch on a sphere.
struct sphere_patch : patch {
	sphere_patch(object *obj, const material &mat, float u1, float u2, float v1, float v2, const vec3 &center, float radius);

	void render(std::vector<patch_vertex> &verts, GLuint patch_index) const;
	vec3 center() const;
	vec3 surface_dir() const;
	float area() const;

private:
	float u1_;
	float u2_;
	float v1_;
	float v2_;
	vec3 center_;
	float radius_;
	float area_;

public:
	// returns subdivisions in u, v directions
	static std::pair<std::size_t, std::size_t> subdivisions(float radius, float desired_area);
};

}