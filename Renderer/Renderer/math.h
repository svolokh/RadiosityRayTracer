#pragma once

#include <array>
#include <initializer_list>

#include "vec3.h"

#define RT_PI 3.14159265f

namespace rt {

float degrees(float radians);
float radians(float degrees);

template <typename T>
inline T clamp(T val, T min, T max) {
	if(val <= min) {
		return min;
	} else if(val >= max) {
		return max;
	} else {
		return val;
	}
}

template <typename Vec>
static float length(const Vec &v) {
	return std::sqrt(length_squared(v));
}

template <typename Vec>
static Vec normalize(const Vec &v) {
	return v/length(v);
}

float triangle_area(const vec3 &a, const vec3 &b, const vec3 &c);

}
