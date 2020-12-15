#pragma once

#include <cstddef>

namespace rt {

// Perlin noise from HW1
float noise(float u, float v, float amplitude, std::size_t min_freq_exp, std::size_t max_freq_exp);

}