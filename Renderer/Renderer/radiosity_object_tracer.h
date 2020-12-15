#pragma once

#include "ray.h"
#include "vec3.h"
#include "scene.h"

#include <utility>
#include <tuple>

namespace rt {

static const auto rot_max_depth(5);

// Ray tracer that uses radiosity as its backend for diffuse information.
// Intersection shaders are not supported by this tracer.
template <bool Interpolate>
struct radiosity_object_tracer {
	radiosity_object_tracer(const scene &scn) :
		scn_(scn)
	{
	}

	vec3 trace(const ray &r) {
		return trace_(r, 0);
	}

private:
	vec3 trace_(const ray &r, std::size_t depth) {
		vec3 L(0.0f, 0.0f, 0.0f);
		primitive *pr;
		intersect_info info;
		if(scn_.intersect(r, info, pr)) {
			auto pos(r.position(info.t));
			vec3 diffuse;
			material mat;
			vec3 normal;
			std::tie(diffuse, mat, normal) = patch_info(pr, info.uv[0], info.uv[1]);
			L += diffuse;
			if(depth < rot_max_depth - 1 && length_squared(mat.spec_color) > std::numeric_limits<float>::epsilon()) {
				vec3 R(normal*2.0f*dot(normal, r.dir*-1.0f) + r.dir);
				ray r_reflect(pos + R*RT_RAY_EPSILON, R);
				auto L_s(trace_(r_reflect, depth+1));
				L_s.r *= info.mat.spec_color.r;
				L_s.g *= info.mat.spec_color.g;
				L_s.b *= info.mat.spec_color.b;
				L += L_s;
			}
		}
		L.r = std::min(L.r, 1.0f);
		L.g = std::min(L.g, 1.0f);
		L.b = std::min(L.b, 1.0f);
		return L;
	}

	const scene &scn_;

	// Returns interpolated radiosity color, material, and normal.
	static std::tuple<vec3, material, vec3> patch_info(primitive *pr, float u, float v) {
		auto &states(*pr->radiosity_patches);
		if(Interpolate) {
			auto x(u*states[0].size()-0.5f);
			auto y(v*states.size()-0.5f);
			auto x1{int(x)};
			auto y1{int(y)};
			float tmp;
			auto xf(std::modf(x, &tmp));
			auto yf(std::modf(y, &tmp));
			int x2, y2;
			if(pr->patches_u_loop) {
				if(std::signbit(xf)) {
					x2 = int(x1 - 1);
					if(x2 < 0) {
						x2 = states[0].size() - 1;
					}
				} else {
					x2 = int(x1 + 1) % states[0].size();
				}
			} else {
				if(std::signbit(xf)) {
					x2 = std::max(0, int(x1 - 1));
				} else {
					x2 = std::min(int(x1 + 1), int(states[0].size() - 1));
				}
			}
			if(pr->patches_v_loop) {
				if(std::signbit(yf)) {
					y2 = int(y1 - 1);
					if(y2 < 0) {
						y2 = states.size() - 1;
					}
				} else {
					y2 = int(y1 + 1) % states.size();
				}
			} else {
				if(std::signbit(yf)) {
					y2 = std::max(0, int(y1 - 1));
				} else {
					y2 = std::min(int(y1 + 1), int(states.size() - 1));
				}
			}
			auto *a(states[y1][x1]);
			auto *b(states[y1][x2]);
			auto *c(states[y2][x1]);
			auto *d(states[y2][x2]);
			xf = std::abs(xf);
			yf = std::abs(yf);
			auto sa((1.0f - yf)*(1.0f - xf));
			auto sb((1.0f - yf)*xf);
			auto sc(yf*(1.0f - xf));
			auto sd(yf*xf);
			auto color(a->energy*sa + b->energy*sb + c->energy*sc + d->energy*sd);
			auto mat(material::interpolate(a->p->mat(), sa, b->p->mat(), sb, c->p->mat(), sc, d->p->mat(), sd));
			auto normal(normalize(a->p->normal()*sa + b->p->normal()*sb + c->p->normal()*sc + d->p->normal()*sd));
			return std::make_tuple(color, mat, normal);
		} else {
			auto x{std::min(int(u*states[0].size()), int(states[0].size() - 1))};
			auto y{std::min(int(v*states.size()), int(states.size() - 1))};
			auto *st(states[y][x]);
			return std::make_tuple(st->energy, st->p->mat(), st->p->normal());
		}
	}
};

}