#pragma once

#include "vec3.h"
#include "ray.h"

#include <array>

namespace rt {

struct aabb {
    aabb(const vec3 &min, const vec3 &max);
	aabb(const aabb &a, const aabb &b);

	bool contains(const vec3 &p) const;
	bool contains(const aabb &o) const; // Checks that AABB o is _entirely_ contained within this AABB (as opposed to overlaps, which checks for any sort of overlap, in addition to complete containment)
	bool overlaps(const aabb &o) const;
    bool intersect(const ray &r, float &t_enter, float &t_exit) const;
	float surface_area() const;

    std::array<vec3, 2> minmax;
};

}