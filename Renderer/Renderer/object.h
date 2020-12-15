#pragma once

#include "scene_io.h"
#include "material.h"
#include "material_shader.h"
#include "intersection_shader.h"

#include <optional.hpp>

#include <string> 
#include <vector>

namespace rt {

using std::experimental::optional;

struct primitive;

struct object {
	object(std::size_t id);

	std::size_t id;
	optional<std::string> name;
	std::vector<material> materials;
	material_shader mat_shader;
	intersection_shader intersect_shader;
	std::vector<primitive *> primitives;
};

}