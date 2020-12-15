#pragma once

#include "vec3.h"
#include "ray.h"
#include "scene.h"
#include "shadow_tracer.h"

#include <vector>

namespace rt {

struct object_tracer {
	object_tracer(const scene &scn);

	vec3 trace(const ray &r);

private:
	vec3 trace_(const ray &r, std::size_t depth, const std::vector<object *> &inside_stack);

private:
	const scene &scn_;
	shadow_tracer st_;
	// A vector of stacks for doing inside/outside tests (one for each depth of recursion).
	// By keeping the stacks here we avoid costly heap allocations: eventually each stack in each depth will grow to the largest reachable stack size (i.e. the deepest level of object nesting in the scene).
	std::vector<std::vector<object *>> inside_stacks_; 
};

}
