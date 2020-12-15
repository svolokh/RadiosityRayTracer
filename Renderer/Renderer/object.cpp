#include "object.h"

namespace rt {

object::object(std::size_t id) :
	id(id),
	mat_shader(&default_material_shader),
	intersect_shader(&default_intersection_shader)
{
}

}