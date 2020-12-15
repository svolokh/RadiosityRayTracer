#pragma once

#include "vec3.h"
#include "material.h"

namespace rt {

struct patch;

struct patch_state {
	patch_state(); // null patch state constructor
	patch_state(const patch *p);

	const patch *p;
	vec3 energy;
	vec3 unshot_energy;
	float unshot_energy_total; 
};

}