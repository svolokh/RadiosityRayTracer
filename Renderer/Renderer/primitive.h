#pragma once

#include "aabb.h"
#include "ray.h"
#include "object.h"
#include "material.h"
#include "intersect_info.h"
#include "patch_state.h"

#include <variant.hpp> 

#include <cstdint>
#include <memory>

namespace rt {

enum primitive_type {
	type_sphere,
	type_triangle
};

struct primitive {
	primitive(primitive_type type, object *obj);
	virtual ~primitive() = default;

	primitive_type type() const;
	object *obj() const;
	virtual bool intersect(const ray &r, intersect_info &info) const = 0;
	virtual const aabb &bounds() const = 0;

public:
	// If using radiosity, contains the patches composing this primitive.
	// A patch containing (u, v) is at index (floor(u*radiosity_patches[0].size()), floor(v*radiosity_patches.size())).
	std::shared_ptr<std::vector<std::vector<patch_state *>>> radiosity_patches;
	bool patches_u_loop; // Whether the radiosity patches are in a loop in the u direction (e.g. a sphere) and can be interpolated across the other side of the grid
	bool patches_v_loop; // Like patches_u_loop but for the v direction

private:
	primitive_type type_;
	object *obj_;
};

}
