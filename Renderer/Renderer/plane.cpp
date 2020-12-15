#include "plane.h"
#include "math.h"

namespace rt {

plane::plane(const vec3 &normal, const vec3 &pos) :
	normal(normal),
	pos(pos),
	d_(dot(normal, pos))
{
}

optional<float> plane::intersect(const ray &r) const {
	auto t((d_ - dot(normal, r.origin))/dot(normal, r.dir));
	if(std::isinf(t) || std::signbit(t)) {
		return {};
	} else {
		return t;
	}
}

}