#include "prng.h"

#include <random>

namespace rt {

prng::prng() {
	std::random_device rd;
	std::seed_seq seq{rd(), rd(), rd(), rd()};
	seq.generate(std::begin(s_), std::end(s_));
}

prng::prng(uint_fast32_t s0, uint_fast32_t s1, uint_fast32_t s2, uint_fast32_t s3) {
	s_[0] = s0;
	s_[1] = s1;
	s_[2] = s2;
	s_[3] = s3;
}

static inline uint32_t rotl(uint32_t x, int k) {
	return (x << k) | (x >> (32 - k));
}

prng::result_type prng::operator()() {
	// xoshiro128+ from http://xoshiro.di.unimi.it/
	auto res(s_[0] + s_[3]);
	auto t(s_[1] << 9);
	s_[2] ^= s_[0];
	s_[3] ^= s_[1];
	s_[1] ^= s_[2];
	s_[0] ^= s_[3];
	s_[2] ^= t;
	s_[3] = rotl(s_[3], 11);
	return res;
}

}