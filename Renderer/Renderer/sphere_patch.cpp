#include "sphere_patch.h"
#include "math.h"
#include "sphere.h"

#include <cmath>
#include <algorithm>

namespace rt {

sphere_patch::sphere_patch(object *obj, const material &mat, float u1, float u2, float v1, float v2, const vec3 &center, float radius) :
	patch(obj, mat, normalize(sphere::pos(center, radius, (u1 + u2)/2.0f, (v1 + v2)/2.0f) - center)),
	u1_(u1),
	u2_(u2),
	v1_(v1),
	v2_(v2),
	center_(center),
	radius_(radius)
{
	auto p1(sphere::pos(center_, radius_, u1_, v1_));
	auto p2(sphere::pos(center_, radius_, u2_, v1_));
	auto p3(sphere::pos(center_, radius_, u2_, v2_));
	auto p4(sphere::pos(center_, radius_, u1_, v2_));
	area_ = triangle_area(p1, p2, p3) + triangle_area(p1, p3, p4);
}

void sphere_patch::render(std::vector<patch_vertex> &verts, GLuint patch_index) const {
	auto div(5);
	auto u_step((u2_ - u1_)/div);
	auto v_step((v2_ - v1_)/div);
	for(std::size_t vi(0); vi != div; ++vi) {
		auto v(v1_ + vi*v_step);
		for(std::size_t ui(0); ui != div; ++ui) {
			auto u(u1_ + ui*u_step);
			verts.push_back(patch_vertex{sphere::pos(center_, radius_, u, v + v_step), patch_index});
			verts.push_back(patch_vertex{sphere::pos(center_, radius_, u, v), patch_index});
			verts.push_back(patch_vertex{sphere::pos(center_, radius_, u + u_step, v), patch_index});
			verts.push_back(patch_vertex{sphere::pos(center_, radius_, u, v + v_step), patch_index});
			verts.push_back(patch_vertex{sphere::pos(center_, radius_, u + u_step, v), patch_index});
			verts.push_back(patch_vertex{sphere::pos(center_, radius_, u + u_step, v + v_step), patch_index});
		}
	}
}

vec3 sphere_patch::center() const {
	return sphere::pos(center_, radius_, (u1_ + u2_)/2.0f, (v1_ + v2_)/2.0f);
}

vec3 sphere_patch::surface_dir() const {
	auto c(center());
	auto u((u1_ + u2_)/2.0f);
	auto p1(sphere::pos(center_, radius_, u, v1_));
	auto p2(sphere::pos(center_, radius_, u, v2_));
	return normalize(p1 + p2 - c*2.0f);
}

float sphere_patch::area() const {
	return area_;
}

std::pair<std::size_t, std::size_t> sphere_patch::subdivisions(float radius, float desired_area) {
	auto k(std::asin(std::sqrt(desired_area)/(2*radius)));
	auto du(1.0f/RT_PI*k);
	auto dv(2.0f/RT_PI*k);
	return {
		std::max(1u, std::size_t(1.0f/du)),
		std::max(1u, std::size_t(1.0f/dv))
	};
}

}