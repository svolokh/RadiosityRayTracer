#pragma once

#include "vec2.h"
#include "vec3.h"

#include <cstdlib>

namespace rt {

struct vec4 {
	vec4();
	vec4(float x, float y, float z, float w);
	vec4(float v);
	vec4(const vec3 &v, float w);
	vec4(const vec2 &v, float z, float w);
	vec4(float *arr);

	vec4 operator +(const vec4 &v) const;
	vec4 &operator +=(const vec4 &v);
	vec4 operator -(const vec4 &v) const;
	vec4 &operator -=(const vec4 &v);
	vec4 operator *(float s) const;
	vec4 &operator *=(float s);
	vec4 operator /(float s) const;
	vec4 &operator /=(float s);
	float &operator [](std::size_t index);
	float operator [](std::size_t index) const;
	bool operator ==(const vec4 &o) const;
	bool operator !=(const vec4 &o) const;

	vec3 xyz() const;
	void to_array(float *arr) const;

	union {
		struct { float x; float y; float z; float w; };
		struct { float r; float g; float b; float a; };
	};
};

float length_squared(const vec4 &v);
float dot(const vec4 &a, const vec4 &b);

}
