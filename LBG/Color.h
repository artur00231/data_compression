#pragma once

#include <cstdint>

struct Color
{
	std::uint8_t red;
	std::uint8_t green;
	std::uint8_t blue;
};

inline bool operator==(const Color& c1, const Color& c2)
{
	return c1.red == c2.red && c1.blue == c2.blue && c1.green == c2.green;
}

inline bool operator!=(const Color& c1, const Color& c2)
{
	return !(c1 == c2);
}

namespace std
{
	template<>
	class hash<Color>
	{
	public:
		std::size_t operator()(const Color& color) const
		{
			std::uint64_t value{ color.red };
			value <<= 8;
			value |= color.green;
			value <<= 8;
			value |= color.blue;

			return std::hash<std::uint64_t>{}(value);
		}
	};
}

inline auto distance(Color c1, Color c2)
{
	auto x1 = (long long)c1.red - (long long)c2.red;
	auto x2 = (long long)c1.green - (long long)c2.green;
	auto x3 = (long long)c1.blue - (long long)c2.blue;
	return (unsigned long long)(x1 * x1) + (unsigned long long)(x2 * x2) + (unsigned long long)(x3 * x3);
}
