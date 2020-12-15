#include "quad_patch.h"
#include "math.h"

#include <iostream>
#include <algorithm>

namespace rt {

quad_patch::quad_patch(object *obj, const material &mat, const vec3 &normal, const std::array<vec3, 4> &verts) :
	patch(obj, mat, normal),
	v_(verts)
{
	area_ = triangle_area(v_[0], v_[1], v_[2]) + triangle_area(v_[0], v_[2], v_[3]);
}

void quad_patch::render(std::vector<patch_vertex> &verts, GLuint patch_index) const {
	verts.push_back(patch_vertex{v_[0], patch_index});
	verts.push_back(patch_vertex{v_[1], patch_index});
	verts.push_back(patch_vertex{v_[2], patch_index});
	verts.push_back(patch_vertex{v_[0], patch_index});
	verts.push_back(patch_vertex{v_[2], patch_index});
	verts.push_back(patch_vertex{v_[3], patch_index});
}

vec3 quad_patch::center() const {
	return (v_[0] + v_[1] + v_[2] + v_[3])/4.0f;
}

vec3 quad_patch::surface_dir() const {
	return normalize((v_[2] + v_[3])/2.0f - (v_[0] + v_[1])/2.0f);
}

float quad_patch::area() const {
	return area_;
}

std::pair<std::size_t, std::size_t> quad_patch::subdivisions(const vec3 &a, const vec3 &b, const vec3 &c, const vec3 &d, float desired_area) {
	auto A(triangle_area(a, b, c) + triangle_area(a, c, d));
	auto du(std::sqrt(desired_area/A));
	auto udiv{std::max(1u, std::size_t(1.0f/du))};
	return {udiv, udiv};
}

}