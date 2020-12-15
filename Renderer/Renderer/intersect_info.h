#pragma once

namespace rt {

struct intersect_info {
	float t;
	vec3 normal;
	vec2 uv;
	material mat;
};

}
