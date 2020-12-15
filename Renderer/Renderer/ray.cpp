#include "ray.h"

namespace rt {

ray::ray(const vec3 &origin, const vec3 &dir) :
	origin(origin),
	dir(dir),
	idir(1.0f/dir.x, 1.0f/dir.y, 1.0f/dir.z)
{
}

vec3 ray::position(float t) const {
	return origin + dir*t;
}

}