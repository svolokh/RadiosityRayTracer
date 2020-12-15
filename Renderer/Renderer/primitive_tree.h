#pragma once

#include "primitive.h"
#include "aabb.h"
#include "intersect_info.h"

#include <optional.hpp>
#include <variant.hpp>

#include <memory>
#include <iosfwd>
#include <vector>

namespace rt {

namespace detail {

using std::experimental::optional;

struct pt_interior_data {
	std::size_t split_axis; // x = 0, y = 1, z = 2 (allows for fast access into a vector)
	float split_pos;
};

struct pt_leaf_data {
	std::vector<primitive *> primitives;
};

typedef mpark::variant<pt_interior_data, pt_leaf_data> pt_node_data;

// A node of a primitive_tree.
struct pt_node {
	bool leaf() const;

	aabb bounds;
	optional<pt_node_data> data;

	std::array<std::unique_ptr<pt_node>, 2> lr;
};

}

// A non-owning kd-tree of primitives.
struct primitive_tree {
	primitive_tree(const std::vector<primitive *> &primitives);

	bool intersect(const ray &r, intersect_info &info, primitive *&pr) const;

private:
	std::unique_ptr<detail::pt_node> root_;
	friend std::ostream &operator <<(std::ostream &os, const primitive_tree &tree);
};

}