#pragma once

#include "vec2.h"
#include "vec3.h"

namespace rt {

struct shader_params {
	const vec3 &pos;
	const vec3 &normal;
	const vec2 &uv;
};

}