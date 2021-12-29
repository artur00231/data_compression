#pragma once

#include <type_traits>
#include <cstdint>
#include <array>
#include <bitset>
#include <vector>

namespace helper
{
	struct NumberSize
	{
		std::array<char, 256> values{};

		constexpr NumberSize()
		{
			values[0] = 0;
			values[1] = 1;
			values[2] = 2;

			for (std::size_t i = 3; i < 256; i++)
			{
				values[i] = 1 + values[i / 2];
			}
		}

		constexpr char getSize(std::uint64_t val) const
		{
			//Get binary length
			//Using lookup table
			std::uint64_t tmp1{}, tmp2{}, tmp3{};
			if (tmp1 = (val >> 32))
			{
				if (tmp2 = (tmp1 >> 16))
				{
					return (tmp3 = tmp2 >> 8) ? 56 + values[tmp3] : 48 + values[tmp2];
				}
				else
				{
					return (tmp2 = tmp1 >> 8) ? 40 + values[tmp2] : 32 + values[tmp1];
				}
			}
			else
			{
				if (tmp1 = (val >> 16))
				{
					return (tmp2 = tmp1 >> 8) ? 24 + values[tmp2] : 16 + values[tmp1];
				}
				else
				{
					return (tmp2 = val >> 8) ? 8 + values[tmp2] : values[val];
				}
			}

			return 0;
		}
	};

	constexpr NumberSize number_size{};

	template<std::size_t N>
	constexpr void fib(std::array<std::uint64_t, N>& arr, int k)
	{
		//Fill array with fibonacci numbers
		arr[0] = 1;
		arr[1] = 2;
		
		for (std::size_t i = 2; i < static_cast<std::size_t>(k); i++)
		{
			arr[i] = arr[i - 1] + arr[i - 2];
		}
	}
}


using E_gamma = std::integral_constant<int, 1>;
using E_delta = std::integral_constant<int, 2>;
using E_omega = std::integral_constant<int, 3>;
using fib = std::integral_constant<int, 4>;

template<typename T>
struct NumbersCoder
{
	static_assert(std::is_same_v<T, E_gamma>, "Invalid coder!");
};

template<>
struct NumbersCoder<E_gamma> 
{
	template<std::size_t N>
	bool encode(std::bitset<N>& data, std::uint64_t& sh, std::uint64_t value)
	{
		std::uint64_t size = helper::number_size.getSize(value);
		auto zero_count = size - 1;

		if (zero_count + size + sh > N)
			return false;

		while (zero_count --> 0)
		{
			data[sh] = 0;
			sh++;
		}

		std::uint64_t curr_sh = 1ull << (size - 1);
		while (size --> 0)
		{
			data[sh] = value & curr_sh;
			curr_sh >>= 1;
			sh++;
		}

		return true;
	}

	template<std::size_t N>
	std::pair<std::uint64_t, bool> decode(std::bitset<N>& data, std::uint64_t& sh)
	{
		std::size_t size{ 0 };
		auto l_sh = sh;

		while (l_sh < data.size() && data[l_sh] == 0)
		{
			size++;
			++l_sh;
		}

		if (l_sh >= data.size())
			return { 0, false };

		if (data.size() - l_sh < size + 1)
			return { 0, false };

		std::uint64_t value{ 0 };

		for (std::size_t i = 0; i <= size; i++, l_sh++)
		{
			value <<= 1;
			value |= data[l_sh];
		}

		sh = l_sh;
		return { value, true };
	}

	const char fill = 0;
};

template<>
struct NumbersCoder<E_delta>
{
	template<std::size_t N>
	bool encode(std::bitset<N>& data, std::uint64_t& sh, std::uint64_t value)
	{
		std::uint64_t size_val = helper::number_size.getSize(value);
		std::uint64_t size_n = helper::number_size.getSize(size_val);
		auto zero_count = size_n - 1;

		if (zero_count + size_val + size_n - 1 + sh > N)
			return false;

		while (zero_count --> 0)
		{
			data[sh] = 0;
			sh++;
		}

		std::uint64_t curr_sh = 1ull << (size_n - 1);
		while (size_n --> 0)
		{
			data[sh] = size_val & curr_sh;
			curr_sh >>= 1;
			sh++;
		}

		curr_sh = 1ull << (size_val - 2);
		size_val--;
		while (size_val --> 0)
		{
			data[sh] = value & curr_sh;
			curr_sh >>= 1;
			sh++;
		}

		return true;
	}

	template<std::size_t N>
	std::pair<std::uint64_t, bool> decode(std::bitset<N>& data, std::uint64_t& sh)
	{
		std::size_t size_n{ 0 };
		auto l_sh = sh;

		while (l_sh < data.size() && data[l_sh] == 0)
		{
			size_n++;
			++l_sh;
		}

		if (l_sh >= data.size())
			return { 0, false };

		if (data.size() - l_sh < size_n + 1)
			return { 0, false };

		std::uint64_t size_val{ 0 };

		for (std::size_t i = 0; i <= size_n; i++, l_sh++)
		{
			size_val <<= 1;
			size_val |= data[l_sh];
		}

		if (data.size() - l_sh < --size_val)
			return { 0, false };

		std::uint64_t value{ 1 };

		for (std::size_t i = 0; i < size_val; i++, l_sh++)
		{
			value <<= 1;
			value |= data[l_sh];
		}

		sh = l_sh;

		return { value, true };
	}

	const char fill = 0;
};

template<>
struct NumbersCoder<E_omega>
{
	NumbersCoder()
	{
		tmp.reserve(1024);
	}

	template<std::size_t N>
	bool encode(std::bitset<N>& data, std::uint64_t& sh, std::uint64_t value)
	{
		tmp.clear();
		auto it = std::back_inserter(tmp);
		std::uint64_t curr_sh{};
		std::uint64_t size{}, size_c{};

		while (value > 1)
		{
			size_c = size = helper::number_size.getSize(value);
			curr_sh = 1;

			while (size --> 0)
			{
				*it = static_cast<bool>(value & curr_sh);
				curr_sh <<= 1;
				it++;
			}

			value = size_c - 1;
		}

		std::reverse(tmp.begin(), tmp.end());
		tmp.push_back(0);

		if (tmp.size() + sh > N)
			return false;

		for (auto&& x : tmp)
		{
			data[sh] = x;
			++sh;
		}

		return true;
	}

	template<std::size_t N>
	std::pair<std::uint64_t, bool> decode(std::bitset<N>& data, std::uint64_t& sh)
	{
		if (sh != N)
		{
			if (data[sh] == 0)
			{
				sh++;
				return { 1, true };
			}
		}

		auto l_sh = sh;
		std::uint64_t to_read = 2;
		std::uint64_t value{};

		for (;;)
		{
			if (to_read + l_sh > N)
				return { 0, false };

			while (to_read-- > 0)
			{
				value <<= 1;
				value |= data[l_sh++];
			}

			if (l_sh == N)
				return { 0, false };

			if (data[l_sh] == 0)
			{
				sh = l_sh + 1;
				return { value, true };
			}

			to_read = value + 1;
			value = 0;
		}

		return { 0, false };
	}

	const char fill = 1;

private:
	std::vector<char> tmp{};
};

template<>
struct NumbersCoder<fib>
{
	constexpr NumbersCoder()
	{
		helper::fib<>(fib_table, 93);
	}

	template<std::size_t N>
	bool encode(std::bitset<N>& data, std::uint64_t& sh, std::uint64_t value)
	{
		tmp.reset();
		tmp[0] = 1;

		auto l_sh = 1;
		auto curr = bsearch(value, 0, 92);
		decltype(curr) next{};

		while (value)
		{
			tmp <<= 1;
			tmp[0] = 1;
			l_sh++;

			value -= fib_table[curr];
			if (value == 0)
			{
				break;
			}

			//binary search for next fibonacci number
			next = bsearch(value, 0, curr);

			auto diff = curr - next - 1;

			while (diff --> 0)
			{
				tmp <<= 1;
				tmp[0] = 0;
				++l_sh;
			}

			curr = next;
		}

		while (curr --> 0)
		{
			tmp <<= 1;
			tmp[0] = 0;
			++l_sh;
		}

		if (l_sh + sh > N)
			return false;

		for (int i = 0; i < l_sh; i++)
		{
			data[sh] = tmp[i];
			sh++;
		}


		if (value == 0)
			return true;

		return false;
	}

	template<std::size_t N>
	std::pair<std::uint64_t, bool> decode(std::bitset<N>& data, std::uint64_t& sh)
	{
		auto l_sh = sh;

		//require 0 to continue reading data
		bool req0{ false };
		bool valid{};
		std::uint64_t value{};

		while (l_sh < N)
		{
			if (req0 && data[l_sh] == 1)
			{
				valid = true;
				break;
			}
			else
			{
				req0 = false;
			}

			if (data[l_sh] == 1)
				req0 = true;
			l_sh++;
		}

		if (!valid)
			return { 0, false };

		auto size = l_sh - sh;
		for (int i = 0; i < size; i++)
		{
			if (data[sh] == 1)
				value += fib_table[i];
			sh++;
		}
		
		sh++;
		return { value, true };
	}

	std::size_t bsearch(std::uint64_t val, std::size_t L, std::size_t P)
	{
		if (P - L < 2)
			return L;

		auto mid = (P + L) / 2;
		if (fib_table[mid] < val)
		{
			return bsearch(val, mid, P);
		}
		else if (fib_table[mid] > val)
		{
			return bsearch(val, L, mid);
		}
		
		return mid;
	}	

	const char fill = 0;

private:
	std::array<std::uint64_t, 93> fib_table{};
	std::bitset<100> tmp{};
};
