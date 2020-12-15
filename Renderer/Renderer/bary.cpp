#include "bary.h"

namespace rt {

vec3 barycentric(const vec2 &p, const vec2 &v1, const vec2 &v2, const vec2 &v3) {
	// https://en.wikipedia.org/wiki/Barycentric_coordinate_system "Conversion between barycentric and Cartesian coordinates"
	auto det((v2.y - v3.y)*(v1.x - v3.x) + (v3.x - v2.x)*(v1.y - v3.y));
	auto l1(((v2.y - v3.y)*(p.x - v3.x) + (v3.x - v2.x)*(p.y - v3.y))/det);
	auto l2(((v3.y - v1.y)*(p.x - v3.x) + (v1.x - v3.x)*(p.y - v3.y))/det);
	auto l3(1.0f - l1 - l2);
	return {l1, l2, l3};
}

bool inside_triangle(const vec3 &bary) {
	return bary.x >= 0.0f && bary.x <= 1.0f
		&& bary.y >= 0.0f && bary.y <= 1.0f
		&& bary.z >= 0.0f && bary.z <= 1.0f;
}

}
