#pragma once

#include "vec2.h"

#include <cstdlib>

namespace rt {

struct vec3 {
	vec3();
	vec3(float x, float y, float z);
	vec3(float v);
	vec3(const vec2 &v, float z);
	vec3(const float *arr);

	vec3 operator +(const vec3 &v) const;
	vec3 &operator +=(const vec3 &v);
	vec3 operator -(const vec3 &v) const;
	vec3 &operator -=(const vec3 &v);
	vec3 operator *(float s) const;
	vec3 &operator *=(float s);
	vec3 operator /(float s) const;
	vec3 &operator /=(float s);
	float &operator [](std::size_t index);
	float operator [](std::size_t index) const;
	bool operator ==(const vec3 &o) const;
	bool operator !=(const vec3 &o) const;

	void to_array(float *arr) const;

	union {
		struct { float x; float y; float z; };
		struct { float r; float g; float b; };
	};
};

float length_squared(const vec3 &v);
float dot(const vec3 &a, const vec3 &b);
vec3 cross(const vec3 &a, const vec3 &b);

}