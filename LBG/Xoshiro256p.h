#pragma once

#include <random>

class xoshiro256pRND
{
public:
	using result_type = std::uint64_t;

	xoshiro256pRND() {
		std::random_device rd{};
		state[0] = rd();
		state[0] <<= 32;
		state[0] |= rd();
		state[1] = rd();
		state[1] <<= 32;
		state[1] |= rd();
		state[2] = rd();
		state[2] <<= 32;
		state[2] |= rd();
		state[3] = rd();
		state[3] <<= 32;
		state[3] |= rd();
	}

	std::uint64_t operator()() {
		const uint64_t result = state[0] + state[3];
		const uint64_t t = state[1] << 17;

		state[2] ^= state[0];
		state[3] ^= state[1];
		state[1] ^= state[2];
		state[0] ^= state[3];

		state[2] ^= t;
		state[3] = (state[3] << 45) | (state[3] >> (64 - 45));

		return result;
	}

	std::uint64_t operator()(std::uint64_t begin, std::uint64_t end)
	{
		if ((end - begin) == 0) return 0;

		const uint64_t result = state[0] + state[3];
		const uint64_t t = state[1] << 17;

		state[2] ^= state[0];
		state[3] ^= state[1];
		state[1] ^= state[2];
		state[0] ^= state[3];

		state[2] ^= t;
		state[3] = (state[3] << 45) | (state[3] >> (64 - 45));

		return (result % (end - begin)) + begin;
	}

	static std::uint64_t max() {
		return std::numeric_limits<std::uint64_t>::max();
	}

	static std::uint64_t min() {
		return std::numeric_limits<std::uint64_t>::min();
	}

	double operator()(double min, double max) {
		const uint64_t result = state[0] + state[3];
		const uint64_t t = state[1] << 17;

		state[2] ^= state[0];
		state[3] ^= state[1];
		state[1] ^= state[2];
		state[0] ^= state[3];

		state[2] ^= t;
		state[3] = (state[3] << 45) | (state[3] >> (64 - 45));

		alignas(std::uint64_t) double dresult = 0.5;
		*(reinterpret_cast<std::uint64_t*>(&dresult)) |= result >> 12;
		dresult -= 0.5;
		dresult *= 2;

		return dresult * (max - min) + min;;
	}

	static xoshiro256pRND rand;

private:
	uint64_t state[4]{};
};

xoshiro256pRND xoshiro256pRND::rand{};