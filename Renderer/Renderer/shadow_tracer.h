#pragma once

#include "scene.h"
#include "light.h"
#include "vec3.h"

namespace rt {

struct shadow_tracer {
	shadow_tracer(const scene &scn);

	vec3 trace(const vec3 &origin, const light &l) const;

	const scene &scn_;
};

}
