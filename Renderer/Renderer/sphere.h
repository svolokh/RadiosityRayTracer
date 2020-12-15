#pragma once

#include "aabb.h"
#include "vec3.h"
#include "ray.h"
#include "object.h"
#include "material.h"
#include "intersect_info.h"
#include "primitive.h"

namespace rt {

struct sphere : primitive {
    sphere(object *obj, const vec3 &center, float radius);

	bool intersect(const ray &r, intersect_info &info) const;
    const aabb &bounds() const;

private:
	void output_result(const ray &r, float t, intersect_info &info) const;

public:
    const vec3 center;
    const float radius;

private:
	const aabb bounds_;

public:
	static vec3 pos(const vec3 &center, float radius, float u, float v);
};

}