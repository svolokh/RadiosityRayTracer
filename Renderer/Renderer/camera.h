#pragma once

#include "vec3.h"

namespace rt {

struct camera {
	vec3 pos;
	vec3 dir;
	float focal_dist;
	vec3 ortho_up;
	float fov;
};

}