#include "noise.h"
#include "vec2.h"

#include <cassert>
#include <random>

namespace rt {

static vec2 gradient(size_t freq, int x, int y) {
	if(x == freq) {
		return gradient(freq, 0, y);
	} else if(y == freq) {
		return gradient(freq, x, 0);
	}
	std::minstd_rand gen(0xdeadbeef * freq + 0xbadf00d * x + 0x13371337 * y);
	std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
	return {dist(gen), dist(gen)};
}

float noise(float u, float v, float amplitude, std::size_t min_freq_exp, std::size_t max_freq_exp) {
	assert(max_freq_exp >= min_freq_exp);
	auto amp_(amplitude);
	float res(0.0f);
	for(auto freq_exp(min_freq_exp); freq_exp <= max_freq_exp; ++freq_exp) {
		auto freq(1 << freq_exp);
		auto x(u*freq);
		auto y(v*freq);
		auto x0{int(x)};
		auto y0{int(y)};
		auto x1{int(x)+1};
		auto y1{int(y)+1};
		auto s_x(3*(x - x0)*(x - x0) - 2*(x - x0)*(x - x0)*(x - x0));
		auto s_y(3*(y - y0)*(y - y0) - 2*(y - y0)*(y - y0)*(y - y0));
		auto ne(dot(vec2(x - x1, y - y1), gradient(freq, x1, y1)));
		auto nw(dot(vec2(x - x0, y - y1), gradient(freq, x0, y1)));
		auto se(dot(vec2(x - x1, y - y0), gradient(freq, x1, y0)));
		auto sw(dot(vec2(x - x0, y - y0), gradient(freq, x0, y0)));
		res += amp_ * ((1.0f - s_x)*((1.0f - s_y)*sw + s_y*nw) + s_x*((1.0f - s_y)*se + s_y*ne));
		amp_ /= 2.0f;
	}
	return res;
}

}