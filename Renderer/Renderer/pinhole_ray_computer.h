#pragma once

#include "ray_computer.h"
#include "camera.h"
#include "vec3.h"

namespace rt {

struct pinhole_ray_computer : ray_computer {
	struct params {
	};

	pinhole_ray_computer(const camera &cam, float aspect, const params &par);

	ray compute_ray(float sx, float sy);

private:
	vec3 E_;
	vec3 M_;
	vec3 X_;
	vec3 Y_;
};

}