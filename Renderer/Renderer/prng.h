#pragma once

#include <cstdint>
#include <limits>

namespace rt {

// A fast pseudo-random number generator compatible with <random>.
// PRNG algorithm is xoshiro128+ from http://xoshiro.di.unimi.it/
struct prng {
	typedef uint_fast32_t result_type;

	prng();
	prng(uint_fast32_t s0, uint_fast32_t s1, uint_fast32_t s2, uint_fast32_t s3);

	result_type operator()();

	static constexpr result_type min() {
		return std::numeric_limits<result_type>::min();
	}

	static constexpr result_type max() {
		return std::numeric_limits<result_type>::max();
	}

private:
	uint_fast32_t s_[4];
};

}
