#pragma once

#include "intersection_shader.h"
#include "material_shader.h"

#include <cstddef>
#include <map>

namespace rt {

// Bindings from object ID to shaders.
typedef std::map<std::size_t, std::pair<intersection_shader, material_shader>> shader_bindings;

}