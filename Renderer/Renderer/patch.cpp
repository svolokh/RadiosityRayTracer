#include "patch.h"

namespace rt {

patch::patch(object *obj, const material &mat, const vec3 &normal) :
	obj_(obj),
	mat_(mat),
	normal_(normal)
{
}

object *patch::obj() const {
	return obj_;
}

const material &patch::mat() const {
	return mat_;
}

vec3 patch::normal() const {
	return normal_;
}

std::unique_ptr<patch_state> patch::new_state() const {
	return std::unique_ptr<patch_state>(new patch_state(this));
}

}