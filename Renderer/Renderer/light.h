#pragma once

#include "vec3.h"

#include <variant.hpp>

namespace rt {

struct point_light_info {
	vec3 pos;
};

struct directional_light_info {
	vec3 dir;
};

typedef mpark::variant<point_light_info, directional_light_info> light_info;

struct light {
	light_info info;
	vec3 color;
};

}