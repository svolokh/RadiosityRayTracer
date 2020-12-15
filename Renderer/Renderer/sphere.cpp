#include "sphere.h"
#include "math.h"

#include <iostream>
#include <cmath>

namespace rt {

sphere::sphere(object *obj, const vec3 &center, float radius) :
	primitive(type_sphere, obj),
	center(center),
	radius(radius),
	bounds_{center - vec3(radius), center + vec3(radius)}
{
}

void sphere::output_result(const ray &r, float t, intersect_info &info) const {
	info.t = t;
	auto p(r.position(t) - center);
	info.normal = normalize(p);
	auto u(1.0f/RT_PI*std::atan2(p.x, p.z));
	if(std::signbit(u)) {
		u = 2.0f + u;
	}
	u /= 2.0f;
	info.uv = {
		u,
		1.0f/RT_PI*std::asin(p.y/radius) + 0.5f
	};
	info.mat = obj()->materials[0];
}

bool sphere::intersect(const ray &r, intersect_info &info) const {
	auto v(r.origin - center);
	const auto b(2.0f*dot(r.dir, r.origin - center));
	const auto c(dot(v, v) - radius*radius);
	const auto d(b*b - 4.0f*c);
	if(std::signbit(d)) {
		return false;
	} else {
		auto s(std::sqrt(d));
		float t;
		std::size_t n(0);
		t = (-b - s)/2.0f;
		if(!std::signbit(t)) {
			output_result(r, t, info);
			return true;
		}
		t = (-b + s)/2.0f;
		if(!std::signbit(t)) {
			output_result(r, t, info);
			return true;
		}
		return false;
	}
}

const aabb &sphere::bounds() const {
	return bounds_;
}

vec3 sphere::pos(const vec3 &center, float radius, float u, float v) {
	auto theta(2.0f*RT_PI*u);
	auto phi(RT_PI*(v - 0.5f));
	return center + vec3(
		radius*std::sin(theta)*std::cos(phi),
		radius*std::sin(phi),
		radius*std::cos(theta)*std::cos(phi));
}

}
