#pragma once

#include "ray.h"

namespace rt {

// For a given point on the image plane, determines the ray to shoot in world space.
struct ray_computer {
	virtual ~ray_computer() = default;

	virtual ray compute_ray(float sx, float sy) = 0;
};

}