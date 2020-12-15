#pragma once

#include "vec2.h"
#include "vec3.h"
#include "material.h"
#include "shader_params.h"

#include <functional>

namespace rt {

typedef std::function<void(material &mat, const shader_params &params)> material_shader;

void default_material_shader(material &mat, const shader_params &params);

}