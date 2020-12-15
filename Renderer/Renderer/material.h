#pragma once

#include "vec3.h"

namespace rt {

struct material {
	vec3 diff_color;
	vec3 amb_color;
	vec3 spec_color;
	vec3 emiss_color;
	float shininess;
	float ktran;

	static material interpolate(const material &m0, float w0, const material &m1, float w1, const material &m2, float w2);
	static material interpolate(const material &m0, float w0, const material &m1, float w1, const material &m2, float w2, const material &m3, float w3);
};

}