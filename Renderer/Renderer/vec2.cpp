#include "vec2.h"

namespace rt {

vec2::vec2() {
}

vec2::vec2(float x, float y) :
	x(x), 
	y(y)
{
}

vec2::vec2(float v) :
	x(v),
	y(v)
{
}

vec2::vec2(const float *arr) :
	x(arr[0]),
	y(arr[1])
{
}

vec2 vec2::operator +(const vec2 &v) const {
	return {
		x + v.x,
		y + v.y
	};
}

vec2 &vec2::operator +=(const vec2 &v) {
	x += v.x;
	y += v.y;
	return *this;
}

vec2 vec2::operator -(const vec2 &v) const {
	return {
		x - v.x,
		y - v.y
	};
}

vec2 &vec2::operator -=(const vec2 &v) {
	x -= v.x;
	y -= v.y;
	return *this;
}

vec2 vec2::operator *(float s) const {
	return {
		x * s,
		y * s
	};
}

vec2 &vec2::operator *=(float s) {
	x *= s;
	y *= s;
	return *this;
}

vec2 vec2::operator /(float s) const {
	return {
		x / s,
		y / s
	};
}

vec2 &vec2::operator /=(float s) {
	x /= s;
	y /= s;
	return *this;
}

float &vec2::operator [](std::size_t index) {
	return (&x)[index];
}

float vec2::operator [](std::size_t index) const {
	return (&x)[index];
}

bool vec2::operator ==(const vec2 &o) const {
	return x == o.x && y == o.y;
}

bool vec2::operator !=(const vec2 &o) const {
	return !(*this == o);
}

void vec2::to_array(float *arr) const {
	arr[0] = x;
	arr[1] = y;
}

float length_squared(const vec2 &v) {
	return v.x*v.x + v.y*v.y;
}

float dot(const vec2 &a, const vec2 &b) {
	return a.x*b.x + a.y*b.y;
}

}
