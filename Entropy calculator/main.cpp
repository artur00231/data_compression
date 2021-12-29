#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <cstddef>
#include <execution>
#include <filesystem>

bool readFile(const std::string& path, std::vector<std::size_t>& characters_count,
	std::array<std::vector<std::size_t>, std::numeric_limits<unsigned char>::max() + 1>& specific_characters_count)
{
	std::fstream in;
	in.open(path, std::ios_base::in | std::ios_base::binary);

	if (!in) return false;

	std::size_t total_size = std::filesystem::file_size(path);
	std::cout << "To load " << total_size << " B; " << total_size / (1024 * 1000) << " MB\n";
	std::size_t load_info_level = 1024 * 100000;
	std::size_t load_info_level_change = 1024 * 100000;


	std::vector<unsigned char> buffer{};
	buffer.resize(1024 * 100);
	unsigned char last_character = 0;
	std::size_t loaded = 0;

	do
	{
		in.read(reinterpret_cast<char*>(buffer.data()), buffer.size());

		if (!in)
		{
			buffer.resize(in.gcount());
		}

		std::for_each(buffer.begin(), buffer.end(), [&characters_count, &specific_characters_count, &last_character](auto&& x) {
			characters_count.at(x)++;
			specific_characters_count.at(last_character).at(x)++;
			last_character = x; });

		loaded += buffer.size();

		if (loaded >= load_info_level)
		{
			std::cout << "Lodaed " << loaded / (1024 * 1000) << " MB\n";
			load_info_level += load_info_level_change;
		}

	} while (in);

	in.close();

	return true;
}

int main(int argc, char **argv)
{
	std::string path = "test.txt";

	if (argc == 2)
	{
		path = argv[1];
	}

	std::vector<std::size_t> characters_count{};
	std::array<std::vector<std::size_t>, std::numeric_limits<unsigned char>::max() + 1> specific_characters_count{};
	characters_count.resize(std::numeric_limits<unsigned char>::max() + 1);
	std::for_each(specific_characters_count.begin(), specific_characters_count.end(), [](auto& x) { x.resize(std::numeric_limits<unsigned char>::max() + 1);  });

	auto success = readFile(path, characters_count, specific_characters_count);

	if (!success)
	{
		std::abort();
	}

	std::size_t size = std::reduce(std::execution::par, characters_count.begin(), characters_count.end());

	std::cout << size << std::endl;

	double tmp = 1;
	for (auto&& x : characters_count)
	{
		if (x == 0) continue;

		tmp *= std::powl(x / (double)size, x / (double)size);

	}

	double H = std::abs(std::log2l(tmp));
	std::cout << "H(X) = " << H << std::endl;

	double tmp2 = 1;
	unsigned char character = 0;
	for (auto it = specific_characters_count.begin(); it != specific_characters_count.end(); it++, character++)
	{
		auto character_size = characters_count.at(character);
		if (character_size == 0)
			continue;

		tmp = 1;
		for (auto&& x : *it)
		{
			if (x == 0) continue;

			tmp *= std::powl(x / (double)character_size, x / (double)character_size);
		}

		tmp2 *= std::powl(tmp, character_size / (double)size);
	}

	double Hw = std::abs(std::log2l(tmp2));
	std::cout << "H(Y|X) = " << Hw << std::endl;
	

	std::cout << "Diff " << H - Hw << std::endl;


	std::system("pause > null");

	return 0;
}