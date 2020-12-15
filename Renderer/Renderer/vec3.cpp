#include "vec3.h"

namespace rt {

vec3::vec3() {
}

vec3::vec3(float x, float y, float z) :
	x(x), 
	y(y),
	z(z)
{
}

vec3::vec3(float v) :
	x(v),
	y(v),
	z(v)
{
}

vec3::vec3(const vec2 &v, float z) :
	x(v.x),
	y(v.y),
	z(z)
{
}

vec3::vec3(const float *arr) :
	x(arr[0]),
	y(arr[1]),
	z(arr[2])
{
}

vec3 vec3::operator +(const vec3 &v) const {
	return {
		x + v.x,
		y + v.y,
		z + v.z
	};
}

vec3 &vec3::operator +=(const vec3 &v) {
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

vec3 vec3::operator -(const vec3 &v) const {
	return {
		x - v.x,
		y - v.y,
		z - v.z
	};
}

vec3 &vec3::operator -=(const vec3 &v) {
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

vec3 vec3::operator *(float s) const {
	return {
		x * s,
		y * s,
		z * s
	};
}

vec3 &vec3::operator *=(float s) {
	x *= s;
	y *= s;
	z *= s;
	return *this;
}

vec3 vec3::operator /(float s) const {
	return {
		x / s,
		y / s,
		z / s
	};
}

vec3 &vec3::operator /=(float s) {
	x /= s;
	y /= s;
	z /= s;
	return *this;
}

float &vec3::operator [](std::size_t index) {
	return (&x)[index];
}

float vec3::operator [](std::size_t index) const {
	return (&x)[index];
}

bool vec3::operator ==(const vec3 &o) const {
	return x == o.x && y == o.y && z == o.z;
}

bool vec3::operator !=(const vec3 &o) const {
	return !(*this == o);
}

void vec3::to_array(float *arr) const {
	arr[0] = x;
	arr[1] = y;
	arr[2] = z;
}

float length_squared(const vec3 &v) {
	return v.x*v.x + v.y*v.y + v.z*v.z;
}

float dot(const vec3 &a, const vec3 &b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

vec3 cross(const vec3 &a, const vec3 &b) {
	return {
		a.y*b.z - a.z*b.y, 
		a.z*b.x - a.x*b.z,
		a.x*b.y - a.y*b.x
	};
}

}
