#include "mat4.h"

#include "math.h"
#include <iostream>

namespace rt {

mat4 mat4::viewport(int width, int height) {
	auto w{float(width)};
	auto h{float(height)};
	return {
		{w/2.0f, 0.0f, 0.0f, 0.0f},
		{0.0f, h/2.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 1.0f, 0.0f},
		{(w-1.0f)/2.0f, (h-1.0f)/2.0f, 0.0f, 1.0f}
	};
}

mat4 mat4::translate(float dx, float dy, float dz) {
	return {
		{1.0f, 0.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 1.0f, 0.0f},
		{dx, dy, dz, 1.0f}
	};
}

mat4 mat4::scale(float sx, float sy, float sz) {
	return {
		{sx, 0.0f, 0.0f, 0.0f},
		{0.0f, sy, 0.0f, 0.0f},
		{0.0f, 0.0f, sz, 0.0f},
		{0.0f, 0.0f, 0.0f, 1.0f}
	};
}

mat4 mat4::rotate(float angle, float x, float y, float z) {
	// https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glRotate.xml
	vec3 v{x, y, z};
	if(std::abs(length_squared(v) - 1.0f) > std::numeric_limits<float>::epsilon()) {
		v = normalize(v);
		x = v.x;
		y = v.y;
		z = v.z;
	}
	float c = std::cos(angle), s = std::sin(angle);
	return {
		{x*x*(1-c)+c, y*x*(1-c)+z*s, x*z*(1-c)-y*s, 0.0f},
		{x*y*(1-c)-z*s, y*y*(1-c)+c, y*z*(1-c)+x*s, 0.0f},
		{x*z*(1-c)+y*s, y*z*(1-c)-x*s, z*z*(1-c)+c, 0.0f},
		{0.0f, 0.0f, 0.0f, 1.0f}
	};
}

mat4 mat4::look_at(const vec3 &eye, const vec3 &center, const vec3 &up) {
	// https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluLookAt.xml
	auto f(normalize(center - eye));
	auto s(normalize(cross(f, up)));
	auto u(cross(s, f));
	mat4 M {
		{s.x, u.x, -f.x, 0.0f},
		{s.y, u.y, -f.y, 0.0f},
		{s.z, u.z, -f.z, 0.0f},
		{0.0f, 0.0f, 0.0f, 1.0f}
	};
	return M * mat4::translate(-eye.x, -eye.y, -eye.z);
}

mat4 mat4::orthographic(float l, float r, float b, float t, float n, float f) {
	return {
		{2.0f/(r-l), 0.0f, 0.0f, 0.0f},
		{0.0f, 2.0f/(t-b), 0.0f, 0.0f},
		{0.0f, 0.0f, 2.0f/(f-n), 0.0f},
		{-(r+l)/(r-l), -(t+b)/(t-b), -(f+n)/(f-n), 1.0f}
	};
}

mat4 mat4::perspective(float fov, float aspect, float n, float f) {
	// https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml
	auto k(1.0f/std::tanf(fov/2.0f));
	return {
		{k/aspect, 0.0f, 0.0f, 0.0f},
		{0.0f, k, 0.0f, 0.0f},
		{0.0f, 0.0f, (f + n)/(n - f), -1.0f},
		{0.0f, 0.0f, 2.0f*f*n/(n - f), 0.0f}
	};
}

mat4::mat4() {
	for(size_t i(0); i != data.size(); ++i) {
		for(size_t j(0); j != data[0].size(); ++j) {
			data[i][j] = i == j ? 1.0f : 0.0f;
		}
	}
}

mat4::mat4(const std::array<std::array<float, 4>, 4> &data) :
	data(data)
{
}

mat4::mat4(const std::initializer_list<std::initializer_list<float>> &list) {
	auto it(list.begin());
	if(data.size() != list.size()) {
		throw std::runtime_error("Wrong shape initializer list");
	}
	for(size_t i(0); i != data.size(); ++i, ++it) {
		if(data[i].size() != it->size()) {
			throw std::runtime_error("Wrong shape initializer list");
		}
		auto jt(it->begin());
		for(size_t j(0); j != data[i].size(); ++j, ++jt) {
			data[i][j] = *jt;
		}
	}
}

mat4::mat4(raw_construct_tag) {
}

mat4 mat4::operator *(const mat4 &m) const {
	auto &M(*this);
	mat4 r{raw_construct_tag()};
	r[0][0] = M[0][0]*m[0][0] + M[1][0]*m[0][1] + M[2][0]*m[0][2] + M[3][0]*m[0][3];
	r[0][1] = M[0][1]*m[0][0] + M[1][1]*m[0][1] + M[2][1]*m[0][2] + M[3][1]*m[0][3];
	r[0][2] = M[0][2]*m[0][0] + M[1][2]*m[0][1] + M[2][2]*m[0][2] + M[3][2]*m[0][3];
	r[0][3] = M[0][3]*m[0][0] + M[1][3]*m[0][1] + M[2][3]*m[0][2] + M[3][3]*m[0][3];
	r[1][0] = M[0][0]*m[1][0] + M[1][0]*m[1][1] + M[2][0]*m[1][2] + M[3][0]*m[1][3];
	r[1][1] = M[0][1]*m[1][0] + M[1][1]*m[1][1] + M[2][1]*m[1][2] + M[3][1]*m[1][3];
	r[1][2] = M[0][2]*m[1][0] + M[1][2]*m[1][1] + M[2][2]*m[1][2] + M[3][2]*m[1][3];
	r[1][3] = M[0][3]*m[1][0] + M[1][3]*m[1][1] + M[2][3]*m[1][2] + M[3][3]*m[1][3];
	r[2][0] = M[0][0]*m[2][0] + M[1][0]*m[2][1] + M[2][0]*m[2][2] + M[3][0]*m[2][3];
	r[2][1] = M[0][1]*m[2][0] + M[1][1]*m[2][1] + M[2][1]*m[2][2] + M[3][1]*m[2][3];
	r[2][2] = M[0][2]*m[2][0] + M[1][2]*m[2][1] + M[2][2]*m[2][2] + M[3][2]*m[2][3];
	r[2][3] = M[0][3]*m[2][0] + M[1][3]*m[2][1] + M[2][3]*m[2][2] + M[3][3]*m[2][3];
	r[3][0] = M[0][0]*m[3][0] + M[1][0]*m[3][1] + M[2][0]*m[3][2] + M[3][0]*m[3][3];
	r[3][1] = M[0][1]*m[3][0] + M[1][1]*m[3][1] + M[2][1]*m[3][2] + M[3][1]*m[3][3];
	r[3][2] = M[0][2]*m[3][0] + M[1][2]*m[3][1] + M[2][2]*m[3][2] + M[3][2]*m[3][3];
	r[3][3] = M[0][3]*m[3][0] + M[1][3]*m[3][1] + M[2][3]*m[3][2] + M[3][3]*m[3][3];
	return r;
}

mat4 &mat4::operator *=(const mat4 &m) {
	return *this = *this * m;
}

std::array<float, 4> &mat4::operator[](size_t index) {
	return data[index];
}

const std::array<float, 4> &mat4::operator[](size_t index) const {
	return data[index];
}

vec4 mat4::operator *(const vec4 &v) const {
	auto &m(*this);
	return {
		m[0][0]*v.x + m[1][0]*v.y + m[2][0]*v.z + m[3][0]*v.w,
		m[0][1]*v.x + m[1][1]*v.y + m[2][1]*v.z + m[3][1]*v.w,
		m[0][2]*v.x + m[1][2]*v.y + m[2][2]*v.z + m[3][2]*v.w,
		m[0][3]*v.x + m[1][3]*v.y + m[2][3]*v.z + m[3][3]*v.w
	};
}

}
