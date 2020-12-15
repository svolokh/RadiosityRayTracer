#pragma once

#include "object.h"
#include "vec3.h"
#include "bary.h"
#include "ray.h"
#include "aabb.h"
#include "material.h"
#include "intersect_info.h"
#include "math.h"
#include "primitive.h"

#include <cstdlib>
#include <iostream>

namespace rt {

template <bool PerVertexNormal>
struct triangle_normal;

template <bool PerVertexMaterial>
struct triangle_material;

struct triangle_normal_base {
	virtual vec3 normal(float alpha, float beta) const = 0;
	virtual vec3 normal(std::size_t vi) const = 0;
};

template <>
struct triangle_normal<false> : triangle_normal_base {
	triangle_normal(const vec3 &normal) :
		normal_(normal)
	{
	}

	vec3 normal(float alpha, float beta) const {
		return normal_;
	}

	vec3 normal(std::size_t vi) const {
		return normal_;
	}

private:
	vec3 normal_;
};

template <>
struct triangle_normal<true> : triangle_normal_base {
	triangle_normal(const vec3 &n0, const vec3 &n1, const vec3 &n2) :
		n0_(n0),
		n1_(n1),
		n2_(n2)
	{
	}

	vec3 normal(float alpha, float beta) const {
		return normalize(interpolate(vec3(1.0f - alpha - beta, alpha, beta), n0_, n1_, n2_)); 
	}

	vec3 normal(std::size_t vi) const {
		switch(vi) {
			case 0:
				return n0_;
			case 1:
				return n1_;
			case 2:
				return n2_;
			default:
				throw std::runtime_error("Unexpected index");
		}
	}

private:
	vec3 n0_;
	vec3 n1_;
	vec3 n2_;
};

struct triangle_material_base {
	virtual material mat(object *obj, float alpha, float beta) const = 0;
	virtual material mat(object *obj, std::size_t vi) const = 0;
};

template <>
struct triangle_material<false> : triangle_material_base {
	material mat(object *obj, float alpha, float beta) const {
		return obj->materials[0];
	}

	material mat(object *obj, std::size_t vi) const {
		return obj->materials[0];
	}
};

template <>
struct triangle_material<true> : triangle_material_base {
	triangle_material(std::size_t i0, std::size_t i1, std::size_t i2) :
		i0_(i0),
		i1_(i1),
		i2_(i2)
	{
	}

	material mat(object *obj, float alpha, float beta) const {
		return material::interpolate(obj->materials[i0_], 1.0f - alpha - beta, obj->materials[i1_], alpha, obj->materials[i2_], beta);
	}

	material mat(object *obj, std::size_t vi) const {
		switch(vi) {
			case 0:
				return obj->materials[i0_];
			case 1:
				return obj->materials[i1_];
			case 2:
				return obj->materials[i2_];
			default:
				throw std::runtime_error("Unexpected index");
		}
	}

private:
	std::size_t i0_;
	std::size_t i1_;
	std::size_t i2_;
};

struct triangle_uv {
	triangle_uv(const vec2 &uv0, const vec2 &uv1, const vec2 &uv2) :
		uvs{uv0, uv1, uv2},
		known(true)
	{
	}

	triangle_uv() :
		uvs{vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec2(0.0f, 0.0f)},
		known(false)
	{
	}

	vec2 uv(float alpha, float beta) const {
		return interpolate(vec3(1.0f - alpha - beta, alpha, beta), uvs[0], uvs[1], uvs[2]);
	}

	std::array<vec2, 3> uvs;
	bool known;
};

struct triangle : primitive {
	triangle(object *obj, const vec3 &a, const vec3 &b, const vec3 &c, std::unique_ptr<triangle_normal_base> &&normal, const triangle_uv &uv, std::unique_ptr<triangle_material_base> &&mat) :
		primitive(type_triangle, obj),
		a(a),
		b(b),
		c(c),
		normal(std::move(normal)),
		uv(uv),
		mat(std::move(mat)),
		bounds_{
			{
				std::min(std::min(a.x, b.x), c.x),
				std::min(std::min(a.y, b.y), c.y),
				std::min(std::min(a.z, b.z), c.z)
			},
			{
				std::max(std::max(a.x, b.x), c.x),
				std::max(std::max(a.y, b.y), c.y),
				std::max(std::max(a.z, b.z), c.z)
			}
		}
	{
		face_normal_ = normalize(cross(b - a, c - a)); 
		d_ = dot(a * -1.0f, face_normal_);
		vec3 absn(std::abs(face_normal_.x), std::abs(face_normal_.y), std::abs(face_normal_.z));
		if(absn.x > absn.y && absn.x > absn.z) {
			i1_ = 1;
			i2_ = 2;
		} else if(absn.y > absn.x && absn.y > absn.z) {
			i1_ = 0;
			i2_ = 2;
		} else { 
			i1_ = 0;
			i2_ = 1;
		}
	}

	bool intersect(const ray &r, intersect_info &info) const {
		auto denom(dot(face_normal_, r.dir));
		if(std::abs(denom) < std::numeric_limits<float>::epsilon()) {
			return false;
		}
		auto t(-(d_ + dot(face_normal_, r.origin))/denom);
		if(std::signbit(t)) {
			return false;
		}
		auto P(r.position(t));
		auto u0(P[i1_] - a[i1_]);
		auto v0(P[i2_] - a[i2_]);
		auto u1(b[i1_] - a[i1_]);
		auto u2(c[i1_] - a[i1_]);
		auto v1(b[i2_] - a[i2_]);
		auto v2(c[i2_] - a[i2_]);
		float alpha, beta;
		if(std::abs(u1) < std::numeric_limits<float>::epsilon()) {
			beta = u0/u2;
			if(0.0f <= beta && beta <= 1.0f) {
				alpha = (v0 - beta*v2)/v1;
			} else {
				return false;
			}
		} else {
			beta = (v0*u1 - u0*v1)/(v2*u1 - u2*v1);
			if(0.0f <= beta && beta <= 1.0f) {
				alpha = (u0 - beta*u2)/u1;
			} else {
				return false;
			}
		}
		if(alpha >= 0.0f && (alpha + beta) <= 1.0f) {
			info.t = t;
			info.normal = normal->normal(alpha, beta);
			info.uv = uv.uv(alpha, beta);
			info.mat = mat->mat(obj(), alpha, beta);
			return true;
		} else {
			return false;
		}
	}

	const aabb &bounds() const {
		return bounds_;
	}

public:
	const vec3 a;
	const vec3 b;
	const vec3 c;
	std::unique_ptr<triangle_normal_base> normal;
	const triangle_uv uv;
	std::unique_ptr<triangle_material_base> mat;

private:
	vec3 face_normal_;
	float d_;
	aabb bounds_;

	// the axis indices of the projection plane for intersection
	std::size_t i1_;
	std::size_t i2_;

};

}