#include "shadow_tracer.h"
#include "math.h"
#include "ray.h"

#include <optional.hpp>

namespace rt {

using std::experimental::optional;

shadow_tracer::shadow_tracer(const scene &scn) :
	scn_(scn)
{
}

struct compute_ray_direction {
	compute_ray_direction(const vec3 &origin) :
		origin_(origin)
	{
	}

	vec3 operator()(const point_light_info &info) const {
		return normalize(info.pos - origin_);
	}

	vec3 operator()(const directional_light_info &info) const {
		return info.dir * -1.0f;
	}

	const vec3 &origin_;
};

struct compute_f_att {
	compute_f_att(const vec3 &origin) :
		origin_(origin)
	{
	}

	float operator()(const point_light_info &info) const {
		const auto c1(0.25f);
		const auto c2(0.1f);
		const auto c3(0.01f);
		auto d(length(info.pos - origin_));
		return std::min(1.0f, 1.0f/(c1 + c2*d + c3*d*d));
	}

	float operator()(const directional_light_info &info) const {
		return 1.0f;
	}

	const vec3 &origin_;
};

struct compute_max_t {
	compute_max_t(const vec3 &origin) :
		origin_(origin)
	{
	}
	
	float operator()(const point_light_info &info) const {
		return length(info.pos - origin_);
	}

	float operator()(const directional_light_info &info) const {
		return std::numeric_limits<float>::infinity();
	}

	const vec3 &origin_;
};

vec3 shadow_tracer::trace(const vec3 &origin, const light &l) const {
	auto f_att(mpark::visit(compute_f_att(origin), l.info));
	auto dir(mpark::visit(compute_ray_direction(origin), l.info));
	float max_t(mpark::visit(compute_max_t(origin), l.info));
	auto shadow(l.color*f_att);
	optional<ray> r;
	r.emplace(origin, dir);
	primitive *pr;
	intersect_info info;
	while(scn_.intersect(*r, info, pr) && info.t <= max_t) {
		auto pos(r->position(info.t));
		shader_params params{pos, info.normal, info.uv};
		if(pr->obj()->intersect_shader(params)) {
			pr->obj()->mat_shader(info.mat, params);
			if(info.mat.ktran < std::numeric_limits<float>::epsilon()) {
				shadow = vec3(0.0f, 0.0f, 0.0f);
				break;
			}
			auto C(info.mat.diff_color*info.mat.ktran);
			shadow.r *= C.r;
			shadow.g *= C.g;
			shadow.b *= C.b;
		}
		r.emplace(pos + r->dir*RT_RAY_EPSILON, r->dir);
	}
	return shadow;
}

}