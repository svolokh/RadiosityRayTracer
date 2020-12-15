#pragma once

#include "vec3.h"
#include "vec4.h"

#include <array>
#include <initializer_list>
#include <cstdint>

namespace rt {

struct mat4 {
private:
	struct raw_construct_tag {
	};

public:
	static mat4 viewport(int width, int height);
	static mat4 translate(float dx, float dy, float dz);
	static mat4 scale(float sx, float sy, float sz);
	static mat4 rotate(float angle, float x, float y, float z);
	static mat4 look_at(const vec3 &eye, const vec3 &center, const vec3 &up);
	static mat4 orthographic(float l, float r, float b, float t, float n, float f);
	static mat4 perspective(float fov, float aspect, float n, float f);

	mat4();
	mat4(const std::array<std::array<float, 4>, 4> &data);

	mat4(const mat4 &) = default;

	template <typename Matrix>
	mat4(const Matrix &mat) {
		for(size_t i(0); i != 4; ++i) {
			for(size_t j(0); j != 4; ++j) {
				data[i][j] = mat[i][j];
			}
		}
	}

	mat4(const std::initializer_list<std::initializer_list<float>> &list);

private:
	mat4(raw_construct_tag);

public:
	mat4 operator *(const mat4 &m) const;
	mat4 &operator *=(const mat4 &m);
	std::array<float, 4> &operator[](size_t index);
	const std::array<float, 4> &operator[](size_t index) const;
	vec4 operator *(const vec4 &v) const;

	std::array<std::array<float, 4>, 4> data;
};

}