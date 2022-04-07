#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <array>
#include <string>
#include <string_view>
#include <unordered_map>
#include <queue>

#include "Xoshiro256p.h"
#include "Color.h"
#include "ColorSectors.h"

struct PositionAVG
{
	std::uint64_t red{};
	std::uint64_t green{};
	std::uint64_t blue{};

	std::uint64_t count{};
	std::uint64_t distortion;
	double utility_index{};
};

using Point = Color;

using perturbation_vector_t = std::tuple<int, int, int>;


bool loadColors(std::string in, std::unordered_map<Color, std::size_t>& colors);
bool transformImage(std::string in, std::string out, const std::unordered_multimap<std::size_t, Color>& partitions, const std::unordered_map<std::size_t, Color>& code_book);

void LBG(std::unordered_multimap<std::size_t, Color>& partitions, std::unordered_map<Color, std::size_t>& colors, std::unordered_map<std::size_t, Color>& code_book, double epsilon);

void splitPoint(Point& origin, Point& second, perturbation_vector_t perturbation_vector);
std::tuple<std::uint64_t, std::uint64_t, std::uint64_t, std::uint64_t> fastRecalculation(std::size_t point1, std::size_t point2, std::unordered_map<std::size_t, Color>& code_book, std::unordered_multimap<std::size_t, Color>& partitions, const std::unordered_map<Color, std::size_t>& colors);

int main(int argc, char **argv)
{
	if (argc < 4)
	{
		std::cout << "Invalid argumants!\n";
		return 0;
	}

	std::size_t num_ouf_colors{};
	int power = std::stoi(std::string{ argv[3] });

	if (power < 0 || power > 24)
	{
		std::cout << "Invalid nimber of colors!\n";
		return 0;
	}

	num_ouf_colors = 1ull << power;

	std::string in{ argv[1] }, out{ argv[2] };

	//Every color in image and it's count
	std::unordered_map<Color, std::size_t> colors{};

	//Code book it -> color
	std::unordered_multimap<std::size_t, Color> partitions{};

	//ID, color
	std::unordered_map<std::size_t, Color> code_book{};
	std::size_t UUID{};

	double epsilon = 0.1;

	if (argc > 4)
	{
		epsilon = std::stod(std::string{ argv[4] });
	}

	auto success = loadColors(in, colors);

	if (!success)
	{
		std::cout << "Cannot load image!\n";
		return 0;
	}

	PositionAVG pos_avg{};

	//Find centre of image colors
	for (auto [color, count] : colors)
	{
		pos_avg.red += color.red * (std::uint64_t)count;
		pos_avg.green += color.green * (std::uint64_t)count;
		pos_avg.blue += color.blue * (std::uint64_t)count;
		pos_avg.count += count;
	}

	Color avg_color{};
	avg_color.red = pos_avg.red / pos_avg.count;
	avg_color.green = pos_avg.green / pos_avg.count;
	avg_color.blue = pos_avg.blue / pos_avg.count;

	//Init coode book
	code_book.insert({ UUID++, avg_color });

	//Ruin LBG for code book size == 1
	LBG(partitions, colors, code_book, epsilon);

	//Set perturbation vector to [5, 5, 5]
	perturbation_vector_t perturbation_vector{ 5, 5, 5 };
	std::vector<std::pair<std::size_t, Point>> to_insert{};

	while (code_book.size() < num_ouf_colors)
	{
		to_insert.clear();

		//Split every code book entry
		for (auto& [uuid, point] : code_book)
		{
			Point new_point{};
			splitPoint(point, new_point, perturbation_vector);

			to_insert.push_back({ UUID++, new_point });
		}

		for (auto&& x : to_insert)
		{
			code_book.insert(x);
		}

		std::cout << "\nResizeing code book to: " << code_book.size() << "\n";

		LBG(partitions, colors, code_book, epsilon);

		//transformImage(in, out + "_temp" + std::to_string(code_book.size()) + ".tga" , partitions, code_book);
	}

	//SAve end image
	success = transformImage(in, out, partitions, code_book);;

	if (!success)
	{
		std::cout << "Cannot load image!\n";
		return 0;
	}

	return 0;
}

class FileIO
{
public:
	FileIO(std::string_view input_file) {
		input.open(input_file.data(), std::ios_base::in | std::ios_base::binary);
	}

	FileIO(const FileIO&) = delete;
	FileIO(FileIO&&) = delete;
	FileIO& operator=(const FileIO&) = delete;
	FileIO& operator=(FileIO&&) = delete;

	~FileIO() {
		input.close();
	}

	bool isValid() const { return static_cast<bool>(input); }

	template <typename T>
	bool read(std::vector<T>& data) {
		if (!input) return false;
		data.resize(input_size);
		input.read(reinterpret_cast<char*>(data.data()), data.size() * sizeof(T));
		if (!input)
		{
			data.resize(input.gcount());
		}

		return true;
	}


private:
	std::fstream input{};

	const std::size_t input_size = 100 * 1000 * 1024;
};

bool loadColors(std::string in, std::unordered_map<Color, std::size_t>& colors)
{
	FileIO file_IO{ in };

	if (!file_IO.isValid())
		return false;

	std::vector<unsigned char> in_buffer{};
	file_IO.read(in_buffer);
	auto it = in_buffer.begin();

	const auto width = *reinterpret_cast<short*>(&in_buffer[12]);
	const auto height = *reinterpret_cast<short*>(&in_buffer[14]);

	std::cout << "Image ID size: " << (int)in_buffer[0] << "\n";
	std::cout << "Color map: " << std::boolalpha << (bool)in_buffer[1] << "\n";
	std::cout << "Image type: " << (int)in_buffer[2] << "\n";
	std::cout << "Color map spec:\n";
	std::cout << "\tFirst Entry Index: " << *reinterpret_cast<short*>(&in_buffer[3]) << "\n";
	std::cout << "\tColor map Length: " << *reinterpret_cast<short*>(&in_buffer[5]) << "\n";
	std::cout << "\tColor map Entry Size: " << (int)in_buffer[7] << "\n";
	std::cout << "Image spec:\n";
	std::cout << "\tX-origin: " << *reinterpret_cast<short*>(&in_buffer[8]) << "\n";
	std::cout << "\tY-origin: " << *reinterpret_cast<short*>(&in_buffer[10]) << "\n";
	std::cout << "\tWidth: " << *reinterpret_cast<short*>(&in_buffer[12]) << "\n";
	std::cout << "\tHeight: " << *reinterpret_cast<short*>(&in_buffer[14]) << "\n";
	std::cout << "\tPixel Depth: " << (int)in_buffer[16] << "\n";
	std::cout << "\tOrigin: " << (int)((in_buffer[17] >> 4) & 1) << " " << (int)((in_buffer[17] >> 5) & 1) << "\n";
	std::cout << "\tAlpha: " << (int)((in_buffer[17] << 4) >> 4) << "\n";

	//Get pointer to first byte of color data
	it += 18;
	it += (int)in_buffer[0];
	it += *reinterpret_cast<short*>(&in_buffer[5]) * (long long)((int)in_buffer[7] / 8);

	std::size_t pixel_count{};
	Color curr_color{};
	colors.clear();

	while (pixel_count < 3ull * width * height)
	{
		if (it == in_buffer.end())
		{
			if (!file_IO.read(in_buffer))
			{
				std::cerr << "File error!\n";
				return false;
			}

			it = in_buffer.begin();

			if (it == in_buffer.end())
			{
				std::cerr << "File error!\n";
				return false;
			}
		}

		auto pixel = pixel_count % 3;


		switch (pixel)
		{
		case 0:
			//colour.b = *it;
			curr_color.blue = *it;

			break;
		case 1:
			//colour.g = *it;
			curr_color.green = *it;

			break;
		case 2:
			//colour.r = *it;
			curr_color.red = *it;

			colors[curr_color]++;

			curr_color = {};

			break;
		}

		++it;
		pixel_count++;
	}

	std::cout << "\tUniqe colors: " << colors.size() << "\n";

	return true;
}

bool transformImage(std::string in, std::string out, const std::unordered_multimap<std::size_t, Color>& partitions, const std::unordered_map<std::size_t, Color>& code_book)
{
	FileIO file_IO{ in };

	if (!file_IO.isValid())
		return false;

	std::fstream out_file{};
	out_file.open(out, std::ios_base::out | std::ios_base::binary);
	if (!out_file)
		return false;

	std::vector<unsigned char> in_buffer{};
	std::vector<unsigned char> out_buffer{};
	constexpr std::size_t max_out_buffer_size = 10 * 1000 * 1024;

	file_IO.read(in_buffer);
	auto it = in_buffer.begin();

	const auto width = *reinterpret_cast<short*>(&in_buffer[12]);
	const auto height = *reinterpret_cast<short*>(&in_buffer[14]);

	std::size_t to_forward{};
	//Get pointer to first byte of color data
	to_forward += 18;
	to_forward += (int)in_buffer[0];
	to_forward += *reinterpret_cast<short*>(&in_buffer[5]) * (long long)((int)in_buffer[7] / 8);

	while (to_forward --> 0)
	{
		out_buffer.push_back(*(it++));
	}

	std::size_t pixel_count{};
	Color curr_color{};

	std::unordered_map<Color, std::size_t> colors_map{};

	//From code book ID -> color
	//To color -> code book ID
	for (auto x : partitions)
	{
		colors_map[x.second] = x.first;
	}

	std::uint64_t MSE{};
	std::uint64_t SNR{};
	std::uint64_t count{};

	while (pixel_count < 3ull * width * height)
	{
		if (it == in_buffer.end())
		{
			if (!file_IO.read(in_buffer))
			{
				std::cerr << "File error!\n";
				return false;
			}

			it = in_buffer.begin();

			if (it == in_buffer.end())
			{
				std::cerr << "File error!\n";
				return false;
			}
		}

		auto pixel = pixel_count % 3;


		switch (pixel)
		{
		case 0:
			//colour.b = *it;
			curr_color.blue = *it;
			SNR += (std::uint64_t)curr_color.blue * curr_color.blue;

			break;
		case 1:
			//colour.g = *it;
			curr_color.green = *it;
			SNR += (std::uint64_t)curr_color.green * curr_color.green;

			break;
		case 2:
			//colour.r = *it;
			curr_color.red = *it;
			SNR += (std::uint64_t)curr_color.red * curr_color.red;

			auto UUID = colors_map.at(curr_color);
			auto new_color = code_book.at(UUID);

			MSE += distance(new_color, curr_color);
			count++;

			out_buffer.push_back(new_color.blue);
			out_buffer.push_back(new_color.green);
			out_buffer.push_back(new_color.red);

			curr_color = {};

			break;
		}

		if (out_buffer.size() > max_out_buffer_size)
		{
			out_file.write((char*)out_buffer.data(), out_buffer.size());
			out_buffer.clear();
		}

		++it;
		pixel_count++;
	}

	while (true)
	{
		if (it == in_buffer.end())
		{
			if (!file_IO.read(in_buffer))
			{
				break;
			}

			it = in_buffer.begin();

			if (it == in_buffer.end())
			{
				break;
			}
		}

		out_buffer.push_back(*(it++));

		if (out_buffer.size() > max_out_buffer_size)
		{
			out_file.write((char*)out_buffer.data(), out_buffer.size());
			out_buffer.clear();
		}
	}

	count *= 3;

	out_file.write((char*)out_buffer.data(), out_buffer.size());
	out_file.close();

	auto real_MSE = std::sqrt((double)MSE / (double)count);

	std::cout << "\nMSE = " << real_MSE << "\n";
	std::cout << "\nSNR = " << ((double)SNR / (double)count) / real_MSE << "\n";
	std::cout << "\nSNR(dB) = " << 10.0 * log10(((double)SNR / (double)count) / real_MSE) << "\n";

	return true;
}

void LBG(std::unordered_multimap<std::size_t, Color>& partitions, std::unordered_map<Color, std::size_t>& colors, std::unordered_map<std::size_t, Color>& code_book, double epsilon)
{
	//Set perturbation vector for empty partitions to [1, 1, 1]
	std::tuple perturbation_vector{ 1, 1, 1 };

	unsigned long long distortion{};
	unsigned long long colors_count{};

	double avg_distortion{ std::numeric_limits<double>::max() };
	double avg_distortion_last{};

	std::unordered_map<std::size_t, PositionAVG> new_code_book_etries{};

	auto comp = [](const auto& x, const auto& y) { return x.first < y.first; };

	//Init ColorSectors
	ColorSectors color_sectors{ ColorSectors::getMode(code_book.size()) };
	for (auto x : code_book)
	{
		color_sectors.insert(x.second, x.first);
	}

	while (std::abs(((avg_distortion_last - avg_distortion) / avg_distortion)) > epsilon)
	{
		//Recalculate partitions
		partitions.clear();
		new_code_book_etries.clear();
		std::priority_queue<std::pair<double, std::size_t>, std::vector<std::pair<double, std::size_t>>, decltype(comp)> new_partitions_utility_index{ comp };

		for (auto [color, count] : colors)
		{
			std::size_t min_UUID{};
			unsigned long long min_distance{ std::numeric_limits<std::size_t>::max() };

			if (code_book.size() <= 4096)
			{
				//For small number of code_book entries brute force is ok
				for (auto [UUID, position] : code_book)
				{
					auto distance_value = distance(color, position);
					if (distance_value < min_distance)
					{
						min_distance = distance_value;
						min_UUID = UUID;
					}
				}
			}
			else
			{
				//Use more advance search
				min_UUID = color_sectors.findNearest(color);
				min_distance = distance(color, code_book[min_UUID]);
			}

			partitions.insert({ min_UUID, color });

			distortion += min_distance * count;
			colors_count += count;

			auto& position_avg = new_code_book_etries[min_UUID];
			position_avg.red += color.red * (std::uint64_t)count;
			position_avg.green += color.green * (std::uint64_t)count;
			position_avg.blue += color.blue * (std::uint64_t)count;
			position_avg.count += count;
			position_avg.distortion += min_distance * count;
		}

		avg_distortion_last = avg_distortion;
		avg_distortion = (double)distortion / (double)colors_count;

		color_sectors.clear(ColorSectors::getMode(code_book.size()));
		//Update centroids (non empty)
		// Calculate utility_index
		for (auto& [UUID, position_avg] : new_code_book_etries)
		{
			if (position_avg.count > 0)
			{
				Point point;
				point.red = position_avg.red / position_avg.count;
				point.green = position_avg.green / position_avg.count;
				point.blue = position_avg.blue / position_avg.count;

				code_book[UUID] = point;
				color_sectors.insert(point, UUID);
			}

			position_avg.utility_index = (position_avg.distortion / position_avg.count) / avg_distortion;
			if (position_avg.utility_index != 0.0)
				new_partitions_utility_index.push({ position_avg.utility_index, UUID });
		}

		//Find empty partitions, ad move codebook entry to conjested region
		for (auto& [UUID, position] : code_book)
		{
			if (new_code_book_etries.count(UUID) == 0)
			{
				if (new_partitions_utility_index.empty() || new_partitions_utility_index.top().first < 0.3)
				{
					color_sectors.insert(position, UUID);
					continue;
				}

				auto big_partition = new_partitions_utility_index.top();
				new_partitions_utility_index.pop();

				auto partition_other = code_book.at(big_partition.second);
				color_sectors.erase(partition_other, big_partition.second);

				distortion -= new_code_book_etries.at(big_partition.second).distortion;

				splitPoint(partition_other, position, perturbation_vector);
				code_book[big_partition.second] = partition_other;
				color_sectors.insert(partition_other, big_partition.second);
				color_sectors.insert(position, UUID);

				//Do fast recalculation to save some time
				//Only compare this two code book entries and their partitions
				auto [d1, c1, d2, c2] = fastRecalculation(UUID, big_partition.second, code_book, partitions, colors);

				distortion += d1;
				distortion += d2;

				avg_distortion = (double)distortion / (double)colors_count;

				//If splitiong wasn't successful, do not attempt it again in this iteration
				if (c2 != 0 && c1 != 0)
				{
					new_partitions_utility_index.push({ ((double)d2 / (double)c2) / avg_distortion, big_partition.second });
					new_code_book_etries[big_partition.second].distortion = d2;

					new_partitions_utility_index.push({ ((double)d1 / (double)c1) / avg_distortion, UUID });
					new_code_book_etries[UUID].distortion = d1;
				}
			}
		}

		//Progress information
		if (code_book.size() > 8000)
		{
			std::cout << ".";
		}
	}
}

void splitPoint(Point& origin, Point& second, perturbation_vector_t perturbation_vector)
{
	//To access all points, change sign of specific elements
	if (xoshiro256pRND::rand(0.0, 1.0) > 0.5)
		std::get<0>(perturbation_vector) = -std::get<0>(perturbation_vector);
	if (xoshiro256pRND::rand(0.0, 1.0) > 0.5)
		std::get<1>(perturbation_vector) = -std::get<1>(perturbation_vector);
	if (xoshiro256pRND::rand(0.0, 1.0) > 0.5)
		std::get<2>(perturbation_vector) = -std::get<2>(perturbation_vector);

	int new_red = origin.red + std::get<0>(perturbation_vector);
	int new_green = origin.green + std::get<1>(perturbation_vector);
	int new_blue = origin.blue + std::get<2>(perturbation_vector);

	new_red = new_red <= 255 ? new_red : 255;
	new_green = new_green <= 255 ? new_green : 255;
	new_blue = new_blue <= 255 ? new_blue : 255;

	second.red = new_red;
	second.green = new_green;
	second.blue = new_blue;

	new_red = origin.red - std::get<0>(perturbation_vector);
	new_green = origin.green - std::get<1>(perturbation_vector);
	new_blue = origin.blue - std::get<2>(perturbation_vector);

	new_red = new_red >= 0 ? new_red : 0;
	new_green = new_green >= 0 ? new_green : 0;
	new_blue = new_blue >= 0 ? new_blue : 0;

	origin.red = new_red;
	origin.green = new_green;
	origin.blue = new_blue;
}

std::tuple<std::uint64_t, std::uint64_t, std::uint64_t, std::uint64_t> fastRecalculation(std::size_t point1, std::size_t point2, std::unordered_map<std::size_t, Color>& code_book, std::unordered_multimap<std::size_t, Color>& partitions, const std::unordered_map<Color, std::size_t>& colors)
{
	std::vector<std::pair<std::size_t, Color>> new_partitions{};

	std::array<PositionAVG, 2> position_avg{};
	std::size_t offset{};

	//first set
	auto [it, end] = partitions.equal_range(point1);

	for (; it != end; it++)
	{
		const auto& color = it->second;

		auto distance1 = distance(color, code_book[point1]);
		auto distance2 = distance(color, code_book[point2]);

		if (distance1 < distance2)
		{
			offset = 0;
			new_partitions.push_back({ point1, color });
		}
		else
		{
			offset = 1;
			new_partitions.push_back({ point2, color });
		}

		unsigned long long count = colors.at(color);

		position_avg[offset].red += color.red * count;
		position_avg[offset].green += color.green * count;
		position_avg[offset].blue += color.blue * count;
		position_avg[offset].count += count;

		position_avg[offset].distortion += (distance1 < distance2 ? distance1 : distance2) * count;
	}

	//Second set
	std::tie(it, end) = partitions.equal_range(point2);

	for (; it != end; it++)
	{
		const auto& color = it->second;

		auto distance1 = distance(color, code_book[point1]);
		auto distance2 = distance(color, code_book[point2]);

		if (distance1 < distance2)
		{
			offset = 0;
			new_partitions.push_back({ point1, color });
		}
		else
		{
			offset = 1;
			new_partitions.push_back({ point2, color });
		}

		unsigned long long count = colors.at(color);

		position_avg[offset].red += color.red * count;
		position_avg[offset].green += color.green * count;
		position_avg[offset].blue += color.blue * count;
		position_avg[offset].count += count;

		position_avg[offset].distortion += (distance1 < distance2 ? distance1 : distance2) * count;
	}

	partitions.erase(point1);
	partitions.erase(point2);

	for (auto&& x : new_partitions)
	{
		partitions.insert(x);
	}

	new_partitions.clear();

	//point1
	if (position_avg[0].count > 0)
	{
		Point point;
		point.red = position_avg[0].red / position_avg[0].count;
		point.green = position_avg[0].green / position_avg[0].count;
		point.blue = position_avg[0].blue / position_avg[0].count;

		code_book[point1] = point;
	}

	//point2
	if (position_avg[1].count > 0)
	{
		Point point;
		point.red = position_avg[1].red / position_avg[1].count;
		point.green = position_avg[1].green / position_avg[1].count;
		point.blue = position_avg[1].blue / position_avg[1].count;

		code_book[point2] = point;
	}

	return { position_avg[0].distortion, position_avg[0].count, position_avg[1].distortion, position_avg[1].count };
}