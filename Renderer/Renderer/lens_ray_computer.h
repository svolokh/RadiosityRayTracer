#pragma once

#include "ray_computer.h"
#include "camera.h"
#include "plane.h"
#include "prng.h"

#include <optional.hpp>

#include <random>

namespace rt {

using std::experimental::optional;

struct lens_ray_computer : ray_computer {
	struct params {
		float r = 1.0f; // Radius of the lens
		float m = 1.0f; // Distance from the image plane to the lens
		float f_offs = 0.0f; // Focal plane distance offset
	};

	lens_ray_computer(const camera &cam, float aspect, const params &par);

	ray compute_ray(float sx, float sy);

private:
	optional<plane> focal_plane_;
	vec3 C_; 
	vec3 M_; 
	vec3 A_;
	vec3 B_;
	vec3 X_; 
	vec3 Y_;
	prng gen_;
	std::uniform_real_distribution<float> dist_;
};

}