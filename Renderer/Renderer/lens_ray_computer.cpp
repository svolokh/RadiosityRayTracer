#include "lens_ray_computer.h"
#include "math.h"

#include <iostream>

namespace rt {

lens_ray_computer::lens_ray_computer(const camera &cam, float aspect, const params &par) :
	dist_(-par.r, par.r)
{
	const auto &V(cam.dir);
	const auto &U(cam.ortho_up);
	C_ = cam.pos;
	M_ = C_ - V*par.m;
	A_ = cross(V, U);
	B_ = cross(A_, V);
	X_ = A_*par.m*std::tan(cam.fov*aspect/2.0f);
	Y_ = B_*par.m*std::tan(cam.fov/2.0f);
	auto f(cam.focal_dist + par.f_offs);
	focal_plane_.emplace(V, C_ + V*f);
}

ray lens_ray_computer::compute_ray(float sx, float sy) {
	auto P(M_ + X_*(2.0f*(1.0f - sx) - 1.0f) + Y_*(2.0f*(1.0f - sy) - 1.0f));
	ray r(P, normalize(C_ - P));
	auto t(focal_plane_->intersect(r));
	auto Q(r.position(*t));
	auto S(C_ + A_*dist_(gen_) + B_*dist_(gen_));
	return {S, normalize(Q - S)};
}

}