#pragma once

#include "Color.h"

#include <unordered_map>
#include <unordered_set>
#include <future>
#include <array>

/*Split 256x256x256 cube into smaller cubes
1 - 32x32x32
2 - 16x16x16
3 - 8x8x8
4 - 4x4x4
*/

class ColorSectors
{
public:
	ColorSectors(std::size_t mode) : mode{ mode } {};

	void insert(Color color, std::size_t uuid);
	void erase(Color color, std::size_t uuid);

	void clear(std::size_t mode);

	std::size_t findNearest(Color color);


	std::size_t getSectorID1(std::size_t x, std::size_t y, std::size_t z)
	{
		return (x / sector_size1) + (y / sector_size1) * num_of_sector_in_row1 + (z / sector_size1) * num_of_sector_in_row1 * num_of_sector_in_row1;
	}

	std::size_t getSectorID2(std::size_t x, std::size_t y, std::size_t z)
	{
		return (x / sector_size2) + (y / sector_size2) * num_of_sector_in_row2 + (z / sector_size2) * num_of_sector_in_row2 * num_of_sector_in_row2;
	}

	std::size_t getSectorID3(std::size_t x, std::size_t y, std::size_t z)
	{
		return (x / sector_size3) + (y / sector_size3) * num_of_sector_in_row3 + (z / sector_size3) * num_of_sector_in_row3 * num_of_sector_in_row3;
	}

	std::size_t getSectorID4(std::size_t x, std::size_t y, std::size_t z)
	{
		return (x / sector_size4) + (y / sector_size4) * num_of_sector_in_row4 + (z / sector_size4) * num_of_sector_in_row4 * num_of_sector_in_row4;
	}

	static std::size_t getMode(std::size_t size)
	{
		if (size >= 4194304)
		{
			return 4;
		}
		else if (size >= 262144)
		{
			return 3;
		}
		else if (size >= 65536)
		{
			return 2;
		}
		else
		{
			return 1;
		}
	}

private:
	std::pair<std::uint64_t, std::size_t> findNearestInSector(Color color, std::size_t sector, const std::unordered_multimap<std::size_t, std::pair<Color, std::size_t>>& sectors);
	void getSectorInDepht1(const std::size_t depht, Color color, std::unordered_set<std::size_t>& to_visit);
	void getSectorInDepht2(const std::size_t depht, Color color, std::unordered_set<std::size_t>& to_visit);
	void getSectorInDepht3(const std::size_t depht, Color color, std::unordered_set<std::size_t>& to_visit);
	void getSectorInDepht4(const std::size_t depht, Color color, std::unordered_set<std::size_t>& to_visit);

	std::unordered_multimap<std::size_t, std::pair<Color, std::size_t>> sectors1{};
	std::size_t sector_size1{ 32 };
	std::size_t num_of_sector_in_row1{ 256 / 32 };

	std::unordered_multimap<std::size_t, std::pair<Color, std::size_t>> sectors2{};
	std::size_t sector_size2{ 16 };
	std::size_t num_of_sector_in_row2{ 256 / 16 };

	std::unordered_multimap<std::size_t, std::pair<Color, std::size_t>> sectors3{};
	std::size_t sector_size3{ 8 };
	std::size_t num_of_sector_in_row3{ 256 / 8 };

	std::unordered_multimap<std::size_t, std::pair<Color, std::size_t>> sectors4{};
	std::size_t sector_size4{ 4 };
	std::size_t num_of_sector_in_row4{ 256 / 4 };

	std::size_t mode{ 1 };
};

inline void ColorSectors::insert(Color color, std::size_t uuid)
{
	std::size_t sector{};

	switch (mode)
	{
	case 1:
		sector = getSectorID1(color.red, color.green, color.blue);
		sectors1.insert({ sector,  std::make_pair(color, uuid) });
		break;
	case 2:
		sector = getSectorID2(color.red, color.green, color.blue);
		sectors2.insert({ sector,  std::make_pair(color, uuid) });
		break;
	case 3:
		sector = getSectorID3(color.red, color.green, color.blue);
		sectors3.insert({ sector,  std::make_pair(color, uuid) });
		break;
	case 4:
		sector = getSectorID4(color.red, color.green, color.blue);
		sectors4.insert({ sector,  std::make_pair(color, uuid) });
		break;
	}
}

inline void ColorSectors::erase(Color color, std::size_t uuid)
{
	auto [it, end] = sectors1.equal_range(getSectorID1(color.red, color.green, color.blue));

	switch (mode)
	{
	case 1:
		for (; it != end; it++)
		{
			if (it->second.first == color && it->second.second == uuid)
			{
				sectors1.erase(it);
				return;
			}
		}
		break;
	case 2:
		std::tie(it, end) = sectors2.equal_range(getSectorID2(color.red, color.green, color.blue));

		for (; it != end; it++)
		{
			if (it->second.first == color && it->second.second == uuid)
			{
				sectors2.erase(it);
				return;
			}
		}
		break;
	case 3:
		std::tie(it, end) = sectors3.equal_range(getSectorID3(color.red, color.green, color.blue));

		for (; it != end; it++)
		{
			if (it->second.first == color && it->second.second == uuid)
			{
				sectors3.erase(it);
				return;
			}
		}
		break;
	case 4:
		std::tie(it, end) = sectors4.equal_range(getSectorID4(color.red, color.green, color.blue));

		for (; it != end; it++)
		{
			if (it->second.first == color && it->second.second == uuid)
			{
				sectors4.erase(it);
				return;
			}
		}
		break;
	}

}

inline void ColorSectors::clear(std::size_t mode)
{
	sectors1.clear();
	sectors2.clear();
	sectors3.clear();
	sectors4.clear();

	this->mode = mode;
}

inline std::size_t ColorSectors::findNearest(Color color)
{
	std::size_t sector_size{};
	auto sectors = std::ref(sectors1);

	if (mode == 3)
	{
		sector_size = sector_size3;
		sectors = std::ref(sectors3);
	}
	else if (mode == 2)
	{
		sector_size = sector_size2;
		sectors = std::ref(sectors2);
	}
	else if (mode == 4)
	{
		sector_size = sector_size4;
		sectors = std::ref(sectors4);
	}
	else if (mode == 1)
	{
		sector_size = sector_size1;
		sectors = std::ref(sectors1);
	}

	std::size_t min_UUID = -1;
	unsigned long long min_distance{ std::numeric_limits<std::size_t>::max() };

	std::unordered_set<std::size_t> to_visit{};

	int over{ 0 };
	int over_end{ 1 };
	bool start_counter{ false };

	//Start comparing from center cube
	//And later expolre further regions
	for (std::size_t depth = 0; over != over_end && depth < 255; depth++)
	{
		if (start_counter)
			over++;

		switch (mode)
		{
		case 1:
			getSectorInDepht1(depth, color, to_visit);
			break;
		case 2:
			getSectorInDepht2(depth, color, to_visit);
			break;
		case 3:
			getSectorInDepht3(depth, color, to_visit);
			break;
		case 4:
			getSectorInDepht4(depth, color, to_visit);
			break;
		}

		for (auto x : to_visit)
		{
			auto [distance, uuid] = findNearestInSector(color, x, sectors.get());

			if (uuid != -1)
			{
				if (distance < min_distance)
				{
					min_distance = distance;
					min_UUID = uuid;

					if (!start_counter)
					{
						
						switch (mode)
						{
						case 1:
							//We need expore all cubes, in which points x distance(x, color) <= distance
							over_end = std::sqrt(distance) + 1.0 - depth * (sector_size - 1);
							break;
						default:
							//Is is faster
							over_end = distance - depth * (sector_size - 1);
							break;
						}
						
						//over_end = std::sqrt(distance) + 1.0 - depth * (sector_size - 1);
					}

					start_counter = true;
				}
			}
		}

		to_visit.clear();
	}

	return min_UUID;
}

inline std::pair<std::uint64_t, std::size_t> ColorSectors::findNearestInSector(Color color, std::size_t sector, const std::unordered_multimap<std::size_t, std::pair<Color, std::size_t>>& sectors)
{
	auto [it, end] = sectors.equal_range(sector);

	if (it == end) return { 0, (std::size_t) - 1 };

	std::size_t min_UUID{};
	unsigned long long min_distance{ std::numeric_limits<std::size_t>::max() };

	for (; it != end; it++)
	{
		auto distance_value = distance(color, it->second.first);
		if (distance_value < min_distance)
		{
			min_distance = distance_value;
			min_UUID = it->second.second;
		}
	}

	return { min_distance, min_UUID };
}

inline void ColorSectors::getSectorInDepht1(const std::size_t depht, Color color, std::unordered_set<std::size_t>& to_visit)
{
	const std::size_t sector_size = sector_size1;

	int red_min = (int)color.red - depht * sector_size;
	int green_min = (int)color.green - depht * sector_size;
	int blue_min = (int)color.blue - depht * sector_size;

	red_min = red_min < 0 ? 0 : red_min;
	green_min = green_min < 0 ? 0 : green_min;
	blue_min = blue_min < 0 ? 0 : blue_min;

	int red_max = (int)color.red + depht * sector_size;
	int green_max = (int)color.green + depht * sector_size;
	int blue_max = (int)color.blue + depht * sector_size;

	red_max = red_max > 255 ? 255 : red_max;
	green_max = green_max > 255 ? 255 : green_max;
	blue_max = blue_max > 255 ? 255 : blue_max;

	for (int x = green_min; x <= green_max; x += sector_size)
	{
		for (int y = blue_min; y <= blue_max; y += sector_size)
		{
			if (x >= 0 && x < 256 && y >= 0 && y < 256)
				to_visit.insert(getSectorID1((std::uint8_t)red_max, (std::uint8_t)x, (std::uint8_t)y));
		}
	}

	for (int x = green_min; x <= green_max; x += sector_size)
	{
		for (int y = blue_min; y <= blue_max; y += sector_size)
		{
			if (x >= 0 && x < 256 && y >= 0 && y < 256)
				to_visit.insert(getSectorID1((std::uint8_t)red_min, (std::uint8_t)x, (std::uint8_t)y));
		}
	}

	for (int x = green_min; x <= green_max; x += sector_size)
	{
		for (int y = red_min; y <= red_max; y += sector_size)
		{
			if (x >= 0 && x < 256 && y >= 0 && y < 256)
				to_visit.insert(getSectorID1((std::uint8_t)y, (std::uint8_t)x, (std::uint8_t)blue_max));
		}
	}

	for (int x = green_min; x <= green_max; x += sector_size)
	{
		for (int y = red_min; y <= red_max; y += sector_size)
		{
			if (x >= 0 && x < 256 && y >= 0 && y < 256)
				to_visit.insert(getSectorID1((std::uint8_t)y, (std::uint8_t)x, (std::uint8_t)blue_min));
		}
	}

	for (int x = blue_min; x <= blue_max; x += sector_size)
	{
		for (int y = red_min; y <= red_max; y += sector_size)
		{
			if (x >= 0 && x < 256 && y >= 0 && y < 256)
				to_visit.insert(getSectorID1((std::uint8_t)y, (std::uint8_t)green_max, (std::uint8_t)x));
		}
	}

	for (int x = blue_min; x <= blue_max; x += sector_size)
	{
		for (int y = red_min; y <= red_max; y += sector_size)
		{
			if (x >= 0 && x < 256 && y >= 0 && y < 256)
				to_visit.insert(getSectorID1((std::uint8_t)y, (std::uint8_t)green_min, (std::uint8_t)x));
		}
	}
}

inline void ColorSectors::getSectorInDepht2(const std::size_t depht, Color color, std::unordered_set<std::size_t>& to_visit)
{
	const std::size_t sector_size = sector_size2;

	int red_min = (int)color.red - depht * sector_size;
	int green_min = (int)color.green - depht * sector_size;
	int blue_min = (int)color.blue - depht * sector_size;

	red_min = red_min < 0 ? 0 : red_min;
	green_min = green_min < 0 ? 0 : green_min;
	blue_min = blue_min < 0 ? 0 : blue_min;

	int red_max = (int)color.red + depht * sector_size;
	int green_max = (int)color.green + depht * sector_size;
	int blue_max = (int)color.blue + depht * sector_size;

	red_max = red_max > 255 ? 255 : red_max;
	green_max = green_max > 255 ? 255 : green_max;
	blue_max = blue_max > 255 ? 255 : blue_max;


	for (int x = green_min; x <= green_max; x += sector_size)
	{
		for (int y = blue_min; y <= blue_max; y += sector_size)
		{
			if (x >= 0 && x < 256 && y >= 0 && y < 256)
				to_visit.insert(getSectorID2((std::uint8_t)red_max, (std::uint8_t)x, (std::uint8_t)y));
		}
	}

	for (int x = green_min; x <= green_max; x += sector_size)
	{
		for (int y = blue_min; y <= blue_max; y += sector_size)
		{
			if (x >= 0 && x < 256 && y >= 0 && y < 256)
				to_visit.insert(getSectorID2((std::uint8_t)red_min, (std::uint8_t)x, (std::uint8_t)y));
		}
	}

	for (int x = green_min; x <= green_max; x += sector_size)
	{
		for (int y = red_min; y <= red_max; y += sector_size)
		{
			if (x >= 0 && x < 256 && y >= 0 && y < 256)
				to_visit.insert(getSectorID2((std::uint8_t)y, (std::uint8_t)x, (std::uint8_t)blue_max));
		}
	}

	for (int x = green_min; x <= green_max; x += sector_size)
	{
		for (int y = red_min; y <= red_max; y += sector_size)
		{
			if (x >= 0 && x < 256 && y >= 0 && y < 256)
				to_visit.insert(getSectorID2((std::uint8_t)y, (std::uint8_t)x, (std::uint8_t)blue_min));
		}
	}

	for (int x = blue_min; x <= blue_max; x += sector_size)
	{
		for (int y = red_min; y <= red_max; y += sector_size)
		{
			if (x >= 0 && x < 256 && y >= 0 && y < 256)
				to_visit.insert(getSectorID2((std::uint8_t)y, (std::uint8_t)green_max, (std::uint8_t)x));
		}
	}

	for (int x = blue_min; x <= blue_max; x += sector_size)
	{
		for (int y = red_min; y <= red_max; y += sector_size)
		{
			if (x >= 0 && x < 256 && y >= 0 && y < 256)
				to_visit.insert(getSectorID2((std::uint8_t)y, (std::uint8_t)green_min, (std::uint8_t)x));
		}
	}
}

inline void ColorSectors::getSectorInDepht3(const std::size_t depht, Color color, std::unordered_set<std::size_t>& to_visit)
{
	const std::size_t sector_size = sector_size3;

	int red_min = (int)color.red - depht * sector_size;
	int green_min = (int)color.green - depht * sector_size;
	int blue_min = (int)color.blue - depht * sector_size;

	red_min = red_min < 0 ? 0 : red_min;
	green_min = green_min < 0 ? 0 : green_min;
	blue_min = blue_min < 0 ? 0 : blue_min;

	int red_max = (int)color.red + depht * sector_size;
	int green_max = (int)color.green + depht * sector_size;
	int blue_max = (int)color.blue + depht * sector_size;

	red_max = red_max > 255 ? 255 : red_max;
	green_max = green_max > 255 ? 255 : green_max;
	blue_max = blue_max > 255 ? 255 : blue_max;


	for (int x = green_min; x <= green_max; x += sector_size)
	{
		for (int y = blue_min; y <= blue_max; y += sector_size)
		{
			if (x >= 0 && x < 256 && y >= 0 && y < 256)
				to_visit.insert(getSectorID3((std::uint8_t)red_max, (std::uint8_t)x, (std::uint8_t)y));
		}
	}

	for (int x = green_min; x <= green_max; x += sector_size)
	{
		for (int y = blue_min; y <= blue_max; y += sector_size)
		{
			if (x >= 0 && x < 256 && y >= 0 && y < 256)
				to_visit.insert(getSectorID3((std::uint8_t)red_min, (std::uint8_t)x, (std::uint8_t)y));
		}
	}

	for (int x = green_min; x <= green_max; x += sector_size)
	{
		for (int y = red_min; y <= red_max; y += sector_size)
		{
			if (x >= 0 && x < 256 && y >= 0 && y < 256)
				to_visit.insert(getSectorID3((std::uint8_t)y, (std::uint8_t)x, (std::uint8_t)blue_max));
		}
	}

	for (int x = green_min; x <= green_max; x += sector_size)
	{
		for (int y = red_min; y <= red_max; y += sector_size)
		{
			if (x >= 0 && x < 256 && y >= 0 && y < 256)
				to_visit.insert(getSectorID3((std::uint8_t)y, (std::uint8_t)x, (std::uint8_t)blue_min));
		}
	}

	for (int x = blue_min; x <= blue_max; x += sector_size)
	{
		for (int y = red_min; y <= red_max; y += sector_size)
		{
			if (x >= 0 && x < 256 && y >= 0 && y < 256)
				to_visit.insert(getSectorID3((std::uint8_t)y, (std::uint8_t)green_max, (std::uint8_t)x));
		}
	}

	for (int x = blue_min; x <= blue_max; x += sector_size)
	{
		for (int y = red_min; y <= red_max; y += sector_size)
		{
			if (x >= 0 && x < 256 && y >= 0 && y < 256)
				to_visit.insert(getSectorID3((std::uint8_t)y, (std::uint8_t)green_min, (std::uint8_t)x));
		}
	}
}

inline void ColorSectors::getSectorInDepht4(const std::size_t depht, Color color, std::unordered_set<std::size_t>& to_visit)
{
	const std::size_t sector_size = sector_size4;

	int red_min = (int)color.red - depht * sector_size;
	int green_min = (int)color.green - depht * sector_size;
	int blue_min = (int)color.blue - depht * sector_size;

	red_min = red_min < 0 ? 0 : red_min;
	green_min = green_min < 0 ? 0 : green_min;
	blue_min = blue_min < 0 ? 0 : blue_min;

	int red_max = (int)color.red + depht * sector_size;
	int green_max = (int)color.green + depht * sector_size;
	int blue_max = (int)color.blue + depht * sector_size;

	red_max = red_max > 255 ? 255 : red_max;
	green_max = green_max > 255 ? 255 : green_max;
	blue_max = blue_max > 255 ? 255 : blue_max;


	for (int x = green_min; x <= green_max; x += sector_size)
	{
		for (int y = blue_min; y <= blue_max; y += sector_size)
		{
			if (x >= 0 && x < 256 && y >= 0 && y < 256)
				to_visit.insert(getSectorID4((std::uint8_t)red_max, (std::uint8_t)x, (std::uint8_t)y));
		}
	}

	for (int x = green_min; x <= green_max; x += sector_size)
	{
		for (int y = blue_min; y <= blue_max; y += sector_size)
		{
			if (x >= 0 && x < 256 && y >= 0 && y < 256)
				to_visit.insert(getSectorID4((std::uint8_t)red_min, (std::uint8_t)x, (std::uint8_t)y));
		}
	}

	for (int x = green_min; x <= green_max; x += sector_size)
	{
		for (int y = red_min; y <= red_max; y += sector_size)
		{
			if (x >= 0 && x < 256 && y >= 0 && y < 256)
				to_visit.insert(getSectorID4((std::uint8_t)y, (std::uint8_t)x, (std::uint8_t)blue_max));
		}
	}

	for (int x = green_min; x <= green_max; x += sector_size)
	{
		for (int y = red_min; y <= red_max; y += sector_size)
		{
			if (x >= 0 && x < 256 && y >= 0 && y < 256)
				to_visit.insert(getSectorID4((std::uint8_t)y, (std::uint8_t)x, (std::uint8_t)blue_min));
		}
	}

	for (int x = blue_min; x <= blue_max; x += sector_size)
	{
		for (int y = red_min; y <= red_max; y += sector_size)
		{
			if (x >= 0 && x < 256 && y >= 0 && y < 256)
				to_visit.insert(getSectorID4((std::uint8_t)y, (std::uint8_t)green_max, (std::uint8_t)x));
		}
	}

	for (int x = blue_min; x <= blue_max; x += sector_size)
	{
		for (int y = red_min; y <= red_max; y += sector_size)
		{
			if (x >= 0 && x < 256 && y >= 0 && y < 256)
				to_visit.insert(getSectorID4((std::uint8_t)y, (std::uint8_t)green_min, (std::uint8_t)x));
		}
	}
}
