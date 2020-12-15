#pragma once

#include <cstdlib>

namespace rt {

struct vec2 {
	vec2();
	vec2(float x, float y);
	vec2(float v);
	vec2(const float *arr);

	vec2 operator +(const vec2 &v) const;
	vec2 &operator +=(const vec2 &v);
	vec2 operator -(const vec2 &v) const;
	vec2 &operator -=(const vec2 &v);
	vec2 operator *(float s) const;
	vec2 &operator *=(float s);
	vec2 operator /(float s) const;
	vec2 &operator /=(float s);
	float &operator [](std::size_t index);
	float operator [](std::size_t index) const;
	bool operator ==(const vec2 &o) const;
	bool operator !=(const vec2 &o) const;

	void to_array(float *arr) const;

	union {
		struct { float x; float y; };
		struct { float r; float b; };
	};
};

float length_squared(const vec2 &v);
float dot(const vec2 &a, const vec2 &b);

}
