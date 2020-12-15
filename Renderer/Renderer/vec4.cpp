#include "vec4.h"

namespace rt {

vec4::vec4() {
}

vec4::vec4(float x, float y, float z, float w) :
	x(x),
	y(y),
	z(z),
	w(w)
{
}

vec4::vec4(float v) :
	x(v),
	y(v),
	z(v),
	w(v)
{
}

vec4::vec4(const vec3 &v, float w) :
	x(v.x),
	y(v.y),
	z(v.z),
	w(w)
{
}

vec4::vec4(const vec2 &v, float z, float w) :
	x(v.x),
	y(v.y),
	z(z),
	w(w)
{
}

vec4::vec4(float *arr) :
	x(arr[0]),
	y(arr[1]),
	z(arr[2]),
	w(arr[3])
{
}

vec4 vec4::operator +(const vec4 &v) const {
	return {
		x + v.x,
		y + v.y,
		z + v.z,
		w + v.w
	};
}

vec4 &vec4::operator +=(const vec4 &v) {
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
	return *this;
}

vec4 vec4::operator -(const vec4 &v) const {
	return {
		x - v.x,
		y - v.y,
		z - v.z,
		w - v.w
	};
}

vec4 &vec4::operator -=(const vec4 &v) {
	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;
	return *this;
}

vec4 vec4::operator *(float s) const {
	return {
		x * s,
		y * s,
		z * s,
		w * s
	};
}

vec4 &vec4::operator *=(float s) {
	x *= s;
	y *= s;
	z *= s;
	w *= s;
	return *this;
}

vec4 vec4::operator /(float s) const {
	return {
		x / s,
		y / s,
		z / s,
		w / s
	};
}

vec4 &vec4::operator /=(float s) {
	x /= s;
	y /= s;
	z /= s;
	w /= s;
	return *this;
}

float &vec4::operator [](std::size_t index) {
	return (&x)[index];
}

float vec4::operator [](std::size_t index) const {
	return (&x)[index];
}

bool vec4::operator ==(const vec4 &o) const {
	return x == o.x && y == o.y && z == o.z && w == o.w;
}

bool vec4::operator !=(const vec4 &o) const {
	return !(*this == o);
}

vec3 vec4::xyz() const {
	return {x, y, z};
}

void vec4::to_array(float *arr) const {
	arr[0] = x;
	arr[1] = y;
	arr[2] = z;
	arr[3] = w;
}

float length_squared(const vec4 &v) {
	return v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w;
}

float dot(const vec4 &a, const vec4 &b) {
	return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}

}