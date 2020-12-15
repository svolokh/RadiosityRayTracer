#pragma once

#include "ray.h"

#include <optional.hpp>

namespace rt {

using std::experimental::optional;

struct plane {
	plane(const vec3 &normal, const vec3 &pos);

	optional<float> intersect(const ray &r) const;

	const vec3 normal;
	const vec3 pos;

private:
	const float d_;
};

}