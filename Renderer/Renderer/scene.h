#pragma once

#include "primitive.h"
#include "object.h"
#include "scene_io.h"
#include "camera.h"
#include "light.h"
#include "ray.h"
#include "primitive_tree.h"
#include "material.h"
#include "vec3.h"
#include "config.h"
#include "intersect_info.h"
#include "shader_bindings.h"

#include <optional.hpp>

#include <vector>
#include <memory>
#include <unordered_map>

namespace rt {

using std::experimental::optional;

struct scene {
	scene(SceneIO *io, const shader_bindings &bindings = {});
	scene(const scene &) = delete;

	bool intersect(const ray &r, intersect_info &info, primitive *&pr) const;

	camera cam;
	std::vector<light> lights;
	std::vector<std::unique_ptr<object>> objects;
	std::vector<std::unique_ptr<primitive>> primitives;

private:
#ifdef RT_TREE
	optional<primitive_tree> tree_;
#endif
};

}
