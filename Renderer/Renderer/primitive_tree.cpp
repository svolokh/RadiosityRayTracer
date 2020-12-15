#include "primitive_tree.h"

#include <iostream>
#include <limits>
#include <vector>

// Estimated cost of traversal.
static auto cost_trav(0.5f);

// Estimated cost of a primitive (sphere or triangle) intersection.
static auto cost_primitive(1.0f);

namespace rt {

namespace detail {

bool pt_node::leaf() const {
	return !lr[0] && !lr[1];
}

}

static std::pair<aabb, aabb> split_aabb(const aabb &bounds, size_t axis, float pos) {
	vec3 split_min;
	vec3 split_max;
	for(std::size_t i(0); i != 3; ++i) {
		if(i == axis) {
			split_min[i] = pos;
			split_max[i] = pos;
		} else {
			split_min[i] = bounds.minmax[0][i];
			split_max[i] = bounds.minmax[1][i];
		}
	}
	return {
		{bounds.minmax[0], split_max},
		{split_min, bounds.minmax[1]}
	};
}

// SAH cost for a split volume
static float sah_cost(float sa_v, float sa_l, float sa_r, std::size_t prims_left, std::size_t prims_right) {
	return cost_trav + sa_l/sa_v*cost_primitive*prims_left + sa_r/sa_v*cost_primitive*prims_right;
}

// SAH cost for a whole (unsplit) volume
static float sah_cost(std::size_t prims) {
	return cost_primitive*prims; 
}

struct split_event {
	enum type : int {
		minus_prim,
		cont_prim,
		plus_prim
	};

	bool operator <(const split_event &o) const {
		return pos < o.pos || pos == o.pos && tp < o.tp;
	}

	type tp;
	float pos;
};

static std::unique_ptr<detail::pt_node> construct(const std::vector<primitive *> &primitives, const aabb &bounds) {
	// "On building fast kd-Trees for Ray Tracing, and on doing that in O(N log N)" Section 4.2 Algorithm 4
	std::unique_ptr<detail::pt_node> node(new detail::pt_node{bounds});
	if(primitives.size() <= 1 || bounds.surface_area() < std::numeric_limits<float>::epsilon()) {
		node->data.emplace(detail::pt_leaf_data{primitives});
		return node;
	}
	size_t best_split_axis(-1);
	float best_split_pos(std::numeric_limits<float>::quiet_NaN());
	auto best_split_side(false); // The side on which to place primitives perpendicular to the split axis: false = left, true = right
	auto best_cost(std::numeric_limits<float>::infinity());
	std::vector<split_event> evts;
	// std::cout << "===" << std::endl;
	for(std::size_t axis(0); axis != 3; ++axis) {
		// std::cout << "axis = " << axis << std::endl;
		evts.clear();
		// std::cout << "bounds.minmax[0] = " << bounds.minmax[0].x << "," << bounds.minmax[0].y << "," << bounds.minmax[0].z << std::endl;
		// std::cout << "bounds.minmax[1] = " << bounds.minmax[1].x << "," << bounds.minmax[1].y << "," << bounds.minmax[1].z << std::endl;
		for(auto *pr : primitives) {
			const auto &pr_bounds(pr->bounds());
			// std::cout << "pr_bounds.minmax[0] = " << pr_bounds.minmax[0].x << "," << pr_bounds.minmax[0].y << "," << pr_bounds.minmax[0].z << std::endl;
			// std::cout << "pr_bounds.minmax[1] = " << pr_bounds.minmax[1].x << "," << pr_bounds.minmax[1].y << "," << pr_bounds.minmax[1].z << std::endl;
			auto min(std::max(pr_bounds.minmax[0][axis], bounds.minmax[0][axis]));
			auto max(std::min(pr_bounds.minmax[1][axis], bounds.minmax[1][axis]));
			if(min == max) {
				evts.emplace_back(split_event{split_event::cont_prim, min});
				// std::cout << "adding event: {cont_prim, " << min << "}" << std::endl;
			} else {
				evts.emplace_back(split_event{split_event::plus_prim, min});
				evts.emplace_back(split_event{split_event::minus_prim, max});
				// std::cout << "adding event: {plus_prim, " << min << "}" << std::endl;
				// std::cout << "adding event: {minus_prim, " << max << "}" << std::endl;
			}
		}
		std::size_t prims_left(0);
		std::size_t prims_cont(0);
		std::size_t prims_right(primitives.size());
		std::sort(evts.begin(), evts.end());
		std::size_t i(0);
		while(i != evts.size()) {
			std::size_t minus_prim(0);
			std::size_t cont_prim(0);
			std::size_t plus_prim(0);
			auto pos(evts[i].pos);
			while(i != evts.size() && evts[i].pos == pos && evts[i].tp == split_event::minus_prim) {
				++minus_prim;
				++i;
			}
			while(i != evts.size() && evts[i].pos == pos && evts[i].tp == split_event::cont_prim) {
				++cont_prim;
				++i;
			}
			while(i != evts.size() && evts[i].pos == pos && evts[i].tp == split_event::plus_prim) {
				++plus_prim;
				++i;
			}
			prims_cont = cont_prim;
			prims_right -= cont_prim;
			prims_right -= minus_prim;
			auto ch(split_aabb(bounds, axis, pos));
			auto sa_v(bounds.surface_area());
			auto sa_l(ch.first.surface_area());
			auto sa_r(ch.second.surface_area());
			auto c_side_left(sah_cost(sa_v, sa_l, sa_r, prims_left + prims_cont, prims_right));
			auto c_side_right(sah_cost(sa_v, sa_l, sa_r, prims_left, prims_right + prims_cont));
			auto side(c_side_right < c_side_left);
			auto cost(std::min(c_side_left, c_side_right));
			// std::cout << pos << ": " << plus_prim << " " << cont_prim << " " << minus_prim << " " << prims_left << " " << prims_cont << " " << prims_right;
			if(cost < best_cost) {
				// std::cout << " (best cost!)";
				best_split_axis = axis;
				best_split_side = side;
				best_split_pos = pos;
				best_cost = cost;
			}
			// std::cout << std::endl;
			prims_left += plus_prim;
			prims_left += cont_prim;
		}
	}
	auto no_split_cost(sah_cost(primitives.size()));
	if(no_split_cost < best_cost) {
		node->data.emplace(detail::pt_leaf_data{primitives});
		return node;
	} else {
		std::vector<primitive *> primitives_left;
		std::vector<primitive *> primitives_right;
		for(auto *pr : primitives) {
			const auto &pr_bounds(pr->bounds());
			auto min(pr_bounds.minmax[0][best_split_axis]);
			auto max(pr_bounds.minmax[1][best_split_axis]);
			if(min == best_split_pos && max == best_split_pos) {
				if(best_split_side) {
					primitives_right.push_back(pr);
				} else {
					primitives_left.push_back(pr);
				}
			} else {
				if(min < best_split_pos) {
					primitives_left.push_back(pr);
				} 
				if(max > best_split_pos) {
					primitives_right.push_back(pr);
				} 
			}
		}
		// std::cout << "===" << std::endl;
		// std::cout << "primitives.size() = " << primitives.size() << std::endl;
		// std::cout << "no_split_cost = " << no_split_cost << ", best_cost = " << best_cost << std::endl;
		// std::cout << "best_split_pos = " << best_split_pos << std::endl;
		// std::cout << "primitives_left.size() = " << primitives_left.size() << std::endl;
		// std::cout << "primitives_right.size() = " << primitives_right.size() << std::endl;
		auto bounds_lr(split_aabb(bounds, best_split_axis, best_split_pos));
		node->data.emplace(detail::pt_interior_data{best_split_axis, best_split_pos});
		node->lr[0] = construct(primitives_left, bounds_lr.first);
		node->lr[1] = construct(primitives_right, bounds_lr.second);
		return node;
	}
}

static std::unique_ptr<detail::pt_node> construct_root(const std::vector<primitive *> &primitives) {
	if(primitives.size() == 0) {
		return nullptr;
	}
	auto bounds((*primitives.begin())->bounds());
	for(auto it(primitives.begin()+1); it != primitives.end(); ++it) {
		bounds = aabb(bounds, (*it)->bounds());
	}
	return construct(primitives, bounds);
}

primitive_tree::primitive_tree(const std::vector<primitive *> &primitives) :
	root_(construct_root(primitives))
{
}

struct stack_entry {
	detail::pt_node *node;
	float t_enter;
	float t_exit;
};

bool primitive_tree::intersect(const ray &r, intersect_info &info, primitive *&pr) const {
	static std::vector<stack_entry> intersect_stack;
	// Traversal algorithm from "Review: Kd-tree Traversal Algorithms for Ray Tracing" Section 3.2
	if(!root_) {
		return false;
	}
	intersect_stack.clear();
	detail::pt_node *node;
	float t_enter;
	float t_exit;
	node = root_.get();
	if(!node->bounds.intersect(r, t_enter, t_exit)) {
		return false;
	}
	intersect_stack.emplace_back(stack_entry{node, t_enter, t_exit});
	while(!intersect_stack.empty()) {
		{
			auto &entry(*intersect_stack.rbegin());
			node = entry.node;
			t_enter = entry.t_enter;
			t_exit = entry.t_exit;
			intersect_stack.pop_back();
		}
		while(!node->leaf()) {
			auto &d(mpark::get<detail::pt_interior_data>(*node->data));
			auto t((d.split_pos - r.origin[d.split_axis])*r.idir[d.split_axis]);
			auto b(r.origin[d.split_axis] > d.split_pos);
			auto near(node->lr[b].get());
			auto far(node->lr[!b].get());
			if(t >= t_exit + RT_RAY_EPSILON || t < 0) {
				node = near;
			} else if(t <= t_enter - RT_RAY_EPSILON) {
				node = far;
			} else {
				intersect_stack.emplace_back(stack_entry{far, t, t_exit});
				node = near;
				t_exit = t;
			}
		}
		auto &d(mpark::get<detail::pt_leaf_data>(*node->data));
		intersect_info info_closest;
		info_closest.t = std::numeric_limits<float>::infinity();
		primitive *pr_closest(nullptr);
		{
			intersect_info info;
			for(auto pr : d.primitives) {
				if(pr->intersect(r, info) && info.t < info_closest.t && info.t >= t_enter - RT_RAY_EPSILON && info.t <= t_exit + RT_RAY_EPSILON) {
					info_closest = info;
					pr_closest = pr;
				}
			}
		}
		if(pr_closest != nullptr) {
			info = info_closest;
			pr = pr_closest;
			return true;
		}
	}
	return false;
}

static void print_node(std::ostream &os, const detail::pt_node &n, int indent) {
	auto print_indent([&]() {
		for(auto i(0); i != indent; ++i) {
			os << "    ";
		}
	});
	print_indent();
	std::cout << "// ptr: " << &n << std::endl;
	print_indent();
	std::cout << "// bounds min: {" << n.bounds.minmax[0].x << "," << n.bounds.minmax[0].y << "," << n.bounds.minmax[0].z << "}" << std::endl;
	print_indent();
	std::cout << "// bounds max: {" << n.bounds.minmax[1].x << "," << n.bounds.minmax[1].y << "," << n.bounds.minmax[1].z << "}" << std::endl;
	print_indent();
	std::cout << "// sa: " << n.bounds.surface_area() << std::endl;
	print_indent();
	if(n.leaf()) {
		auto &d(mpark::get<detail::pt_leaf_data>(*n.data));
		for(auto prim : d.primitives) {
			os << prim << " {" << std::endl;
			print_indent(); os << "    ";
			std::cout << "// bounds min: {" << prim->bounds().minmax[0].x << "," << prim->bounds().minmax[0].y << "," << prim->bounds().minmax[0].z << "}" << std::endl;
			print_indent(); os << "    ";
			std::cout << "// bounds max: {" << prim->bounds().minmax[1].x << "," << prim->bounds().minmax[1].y << "," << prim->bounds().minmax[1].z << "}" << std::endl;
			print_indent(); os << "    ";
			std::cout << "// sa: " << prim->bounds().surface_area() << std::endl;
			print_indent();
			os << "}" << std::endl;
			print_indent();
		}
	} else {
		auto &d(mpark::get<detail::pt_interior_data>(*n.data));
		os << "split ";
		switch(d.split_axis) {
			case 0:
				os << "x";
				break;
			case 1:
				os << "y";
				break;
			case 2:
				os << "z";
				break;
			default:
				assert(!"Unexpected split axis");
		}
		os << " @ " << d.split_pos << " -> {" << std::endl;
		print_node(os, *n.lr[0], indent + 1);
		os << std::endl;
		print_indent();
		os << "}, {" << std::endl;
		print_node(os, *n.lr[1], indent + 1);
		os << std::endl;
		print_indent();
		os << "}";
	}
}

std::ostream &operator <<(std::ostream &os, const primitive_tree &tree) {
	if(tree.root_) {
		print_node(os, *tree.root_, 0);
	} else {
		os << "(empty)";
	}
	return os;
}

}