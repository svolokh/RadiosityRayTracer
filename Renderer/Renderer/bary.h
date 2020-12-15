#pragma once

#include "vec2.h"
#include "vec3.h"

namespace rt {

vec3 barycentric(const vec2 &p, const vec2 &v1, const vec2 &v2, const vec2 &v3);
bool inside_triangle(const vec3 &bary);

template <typename T>
T interpolate(const vec3 &bary, const T &o1, const T &o2, const T &o3) {
	return o1 * bary.x + o2 * bary.y + o3 * bary.z;
}

}
