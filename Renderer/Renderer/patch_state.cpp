#include "patch_state.h"
#include "patch.h"

namespace rt {

patch_state::patch_state() :
	p(nullptr),
	energy(0.0f),
	unshot_energy(0.0f),
	unshot_energy_total(0.0f)
{
}

patch_state::patch_state(const patch *p) :
	p(p)
{
	auto emiss(p->mat().emiss_color);
	auto diff(p->mat().diff_color);
	energy = {
		diff.r * emiss.r,
		diff.g * emiss.g,
		diff.b * emiss.b
	};
	unshot_energy = energy;
	unshot_energy_total = energy.r + energy.g + energy.b;
}

}