#pragma once

#include "vec3.h"
#include "config.h"

namespace rt {

struct ray {
    ray(const vec3 &origin, const vec3 &dir);

	vec3 position(float t) const;

    const vec3 origin;
    const vec3 dir;
    const vec3 idir;
};

}