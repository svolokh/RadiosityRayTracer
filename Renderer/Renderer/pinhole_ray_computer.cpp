#include "pinhole_ray_computer.h"
#include "math.h"

#include <cmath>

namespace rt {

pinhole_ray_computer::pinhole_ray_computer(const camera &cam, float aspect, const params &par) {
	const auto c(cam.focal_dist);
	E_ = cam.pos;
	const auto &V(cam.dir);
	const auto &U(cam.ortho_up);
	auto A(cross(V, U));
	auto B(cross(A, V));
	M_ = E_ + V*c;
	X_ = A*c*std::tan(cam.fov*aspect/2.0f);
	Y_ = B*c*std::tan(cam.fov/2.0f);
}

ray pinhole_ray_computer::compute_ray(float sx, float sy) {
	auto P(M_ + X_*(2.0f*sx - 1.0f) + Y_*(2.0f*sy - 1.0f));
	return {E_, normalize(P - E_)};
}

}