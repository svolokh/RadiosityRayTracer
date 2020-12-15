#pragma once

#include <functional>

#include "shader_params.h"

namespace rt {

typedef std::function<bool(const shader_params &params)> intersection_shader;

bool default_intersection_shader(const shader_params &params);

}