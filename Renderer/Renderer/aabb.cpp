#include "aabb.h"

namespace rt {

aabb::aabb(const vec3 &min, const vec3 &max) :
	minmax{min, max}
{
}

aabb::aabb(const aabb &a, const aabb &b) :
	minmax{vec3{
		std::min(a.minmax[0].x, b.minmax[0].x),
		std::min(a.minmax[0].y, b.minmax[0].y),
		std::min(a.minmax[0].z, b.minmax[0].z),
	}, vec3{
		std::max(a.minmax[1].x, b.minmax[1].x),
		std::max(a.minmax[1].y, b.minmax[1].y),
		std::max(a.minmax[1].z, b.minmax[1].z)
	}}
{
}

bool aabb::contains(const vec3 &p) const {
	return minmax[0].x <= p.x && p.x <= minmax[1].x &&
	       minmax[0].y <= p.y && p.y <= minmax[1].y &&
		   minmax[0].z <= p.z && p.z <= minmax[1].z;
}

bool aabb::contains(const aabb &o) const {
	return    minmax[0].x <= o.minmax[0].x && o.minmax[1].x <= minmax[1].x
		   && minmax[0].y <= o.minmax[0].y && o.minmax[1].y <= minmax[1].y
		   && minmax[0].z <= o.minmax[0].z && o.minmax[1].z <= minmax[1].z;
} 

bool aabb::overlaps(const aabb &o) const {
	auto d1(o.minmax[0] - minmax[1]);
	auto d2(minmax[0] - o.minmax[1]);
	return d1.x <= 0.0f && d1.y <= 0.0f && d1.z <= 0.0f 
		&& d2.x <= 0.0f && d2.y <= 0.0f && d2.z <= 0.0f;
}

bool aabb::intersect(const ray &r, float &t_enter, float &t_exit) const {
	// "An Efficient and Robust Ray-Box Intersection Algorithm"
	float tmin;
	float tmax;
	float tmin2;
	float tmax2;
	tmin = (minmax[std::signbit(r.dir.x)].x - r.origin.x)*r.idir.x;
	tmax = (minmax[!std::signbit(r.dir.x)].x - r.origin.x)*r.idir.x;
	tmin2 = (minmax[std::signbit(r.dir.y)].y - r.origin.y)*r.idir.y;
	tmax2 = (minmax[!std::signbit(r.dir.y)].y - r.origin.y)*r.idir.y;
	if(tmax2 < tmin || tmax < tmin2) {
		return false;
	}
	if(tmin2 > tmin) {
		tmin = tmin2;
	}
	if(tmax2 < tmax) {
		tmax = tmax2;
	}
	tmin2 = (minmax[std::signbit(r.dir.z)].z - r.origin.z)*r.idir.z;
	tmax2 = (minmax[!std::signbit(r.dir.z)].z - r.origin.z)*r.idir.z;
	if(tmax2 < tmin || tmax < tmin2) {
		return false;
	}
	if(tmin2 > tmin) {
		tmin = tmin2;
	}
	if(tmax2 < tmax) {
		tmax = tmax2;
	}
	if(std::signbit(tmax)) {
		return false;
	}
	// If tmin < 0 && tmax > 0, then the ray started inside the AABB.
	// Set t_enter to 0 in this case.
	t_enter = std::signbit(tmin) ? 0.0f : tmin;
	t_exit = tmax;
	return true;
}

float aabb::surface_area() const {
	auto xl(minmax[1].x - minmax[0].x);
	auto yl(minmax[1].y - minmax[0].y);
	auto zl(minmax[1].z - minmax[0].z);
	return 2.0f*xl*yl + 2.0f*xl*zl + 2.0f*yl*zl;
}

}
