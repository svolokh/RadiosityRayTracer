#pragma once

#include "patch.h"

#include <array>
#include <cstdint>

namespace rt {

struct quad_patch : patch {
	quad_patch(object *obj, const material &mat, const vec3 &normal, const std::array<vec3, 4> &verts);

	void render(std::vector<patch_vertex> &verts, GLuint patch_index) const;
	vec3 center() const;
	vec3 surface_dir() const;
	float area() const;

private:
	float area_;
	std::array<vec3, 4> v_;

public:
	static std::pair<std::size_t, std::size_t> subdivisions(const vec3 &a, const vec3 &b, const vec3 &c, const vec3 &d, float desired_area);
};

}