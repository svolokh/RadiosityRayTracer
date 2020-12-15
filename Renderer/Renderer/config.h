#pragma once

#include <cassert>

// Use a kd-tree for testing primitive intersection instead of brute-force
#define RT_TREE

// Epsilon value used with rays for preventing self-collisions and intersection with AABBs (see primitive_tree::intersect)
#define RT_RAY_EPSILON 10e-4f