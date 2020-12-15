#include "math.h"

#include <cmath>

namespace rt {

float degrees(float radians) {
	return radians * 180.0f / RT_PI;
}

float radians(float degrees) {
	return degrees * RT_PI / 180.0f;
}

float triangle_area(const vec3 &a, const vec3 &b, const vec3 &c) {
	return length(cross(b - a, c - a))/2.0f;
}

}
