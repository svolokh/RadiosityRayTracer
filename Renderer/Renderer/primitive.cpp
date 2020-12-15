#include "primitive.h"

namespace rt {

primitive::primitive(primitive_type type, object *obj) :
	type_(type),
	obj_(obj),
	patches_u_loop(false),
	patches_v_loop(false)
{
}

primitive_type primitive::type() const {
	return type_;
}

object *primitive::obj() const {
	return obj_;
}

}