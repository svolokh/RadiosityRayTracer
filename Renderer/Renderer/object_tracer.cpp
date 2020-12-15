#include "object_tracer.h"
#include "math.h"

#include <optional.hpp>

#include <iostream>

namespace rt {

static const auto max_depth(5);
static const auto q_scale(32.0f); // shininess scale
static const auto air_refract_index(1.0f);
static const auto obj_refract_index(1.5f);

using std::experimental::optional;

object_tracer::object_tracer(const scene &scn) :
	scn_(scn),
	st_(scn)
{
	inside_stacks_.resize(max_depth);
}

struct compute_light_direction {
	compute_light_direction(const vec3 &pos) :
		pos_(pos)
	{
	}

	vec3 operator()(const point_light_info &info) const {
		return normalize(info.pos - pos_);
	}

	vec3 operator()(const directional_light_info &info) const {
		return info.dir * -1.0f;
	}

	const vec3 &pos_;
};

vec3 object_tracer::trace(const ray &r) {
	inside_stacks_[0].clear();
	return trace_(r, 0, inside_stacks_[0]);
}

vec3 object_tracer::trace_(const ray &r, std::size_t depth, const std::vector<object *> &inside_stack) {
	vec3 L(0.0f, 0.0f, 0.0f);
	primitive *pr;
	intersect_info info;
	if(scn_.intersect(r, info, pr)) {
		auto pos(r.position(info.t));
		L += vec3(info.mat.amb_color.r*info.mat.diff_color.r, info.mat.amb_color.g*info.mat.diff_color.g, info.mat.amb_color.b*info.mat.diff_color.b)*(1.0f - info.mat.ktran);
		auto normal(info.normal);
		shader_params params{pos, normal, info.uv};
		if(!pr->obj()->intersect_shader(params)) {
			ray r_next(pos + r.dir*RT_RAY_EPSILON, r.dir);
			return trace_(r_next, depth, inside_stack);
		}
		pr->obj()->mat_shader(info.mat, params);
		auto inside(!inside_stack.empty() && inside_stack.back() == pr->obj());
		if(inside) {
			normal *= -1.0f;
		}
		for(auto &light : scn_.lights) {
			auto light_dir(mpark::visit(compute_light_direction(pos), light.info));
			auto n_opp(dot(r.dir, normal) > 0.0f ? normal * -1.0f : normal); // Normal that always faces opposing to the ray direction
			auto shadow_trace_pos(dot(n_opp, light_dir) < 0.0f ? pos - light_dir*RT_RAY_EPSILON : pos + light_dir*RT_RAY_EPSILON); // If the normal (opposing the ray direction) faces away from the light direction, then begin shadow tracing from the front of the object (this handles the case where the light source is behind the object)
			auto shadow(st_.trace(shadow_trace_pos, light)); 
			auto ndl(dot(normal, light_dir));
			auto diffuse(info.mat.diff_color*std::max(ndl, 0.0f)*(1.0f - info.mat.ktran));
			auto reflect_dir(normal*2.0f*ndl - light_dir);
			auto d(dot(reflect_dir, r.dir * -1.0f));
			auto specular(d <= 0.0f ? 0.0f : info.mat.spec_color*std::pow(d, q_scale*info.mat.shininess));
			auto Lj(diffuse + specular);
			L += vec3(shadow.r * Lj.r, shadow.g * Lj.g, shadow.b * Lj.b);
		}
		if(depth < max_depth - 1) {
			if(length_squared(info.mat.spec_color) > std::numeric_limits<float>::epsilon()) {
				vec3 R(normal*2.0f*dot(normal, r.dir*-1.0f) + r.dir);
				ray r_reflect(pos + R*RT_RAY_EPSILON, R);
				auto L_s(trace_(r_reflect, depth+1, inside_stack));
				L_s.r *= info.mat.spec_color.r;
				L_s.g *= info.mat.spec_color.g;
				L_s.b *= info.mat.spec_color.b;
				L += L_s;
			}
			if(info.mat.ktran > std::numeric_limits<float>::epsilon()) {
				auto &next_stack(inside_stacks_[depth+1]);
				next_stack.clear();
				float from_index;
				float to_index;
				if(inside) {
					std::copy(inside_stack.begin(), inside_stack.end()-1, std::back_inserter(next_stack));
					from_index = obj_refract_index;
					to_index = next_stack.empty() ? air_refract_index : obj_refract_index;
				} else {
					std::copy(inside_stack.begin(), inside_stack.end(), std::back_inserter(next_stack));
					next_stack.push_back(pr->obj());
					from_index = inside_stack.empty() ? air_refract_index : obj_refract_index;
					to_index = obj_refract_index;
				}
				auto I(r.dir * -1.0f);
				auto ndi(dot(normal, I));
				auto det(from_index*from_index/(to_index*to_index)*(1.0f - ndi*ndi));
				if(det <= 1.0f) {
					auto Q(normal * ndi);
					auto T((Q - I)*from_index/to_index - normal*std::sqrt(1 - det));
					ray r_transmit(pos + T*RT_RAY_EPSILON, T);
					auto L_t(trace_(r_transmit, depth+1, next_stack));
					L += L_t*info.mat.ktran;
				}
			}
		}
	}
	L.r = std::min(L.r, 1.0f);
	L.g = std::min(L.g, 1.0f);
	L.b = std::min(L.b, 1.0f);
	return L;
}

}
