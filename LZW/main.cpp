#include <iostream>
#include <random>
#include <vector>
#include <list>
#include <optional>
#include <fstream>
#include <numeric>
#include <algorithm>
#include <iterator>

#include "numberscoder.h"
#include "HashTable.h"


namespace ProgramSettings
{
	//Size of hash table
	constexpr std::size_t DHT_base_size = 10 * 1000ull * 1024ull;
}

template <typename NC, typename SPEED>
std::optional<std::tuple<std::uint64_t, std::size_t, double, double, double, double>> code(std::string_view input_file, std::string_view output_file);
using code_function = std::optional<std::tuple<std::uint64_t, std::uint64_t, double, double, double, double>>(*)(std::string_view, std::string_view);

template <typename NC, typename SPEED>
bool decode(std::string_view input_file, std::string_view output_file);
using decode_function = bool(*)(std::string_view, std::string_view);

template <typename SPEED>
code_function getCodeFunction(std::string_view NC)
{
	if (NC == "gamma")
		return code<NumbersCoder<E_gamma>, SPEED>;
	if (NC == "delta")
		return code<NumbersCoder<E_delta>, SPEED>;
	if (NC == "omega")
		return code<NumbersCoder<E_omega>, SPEED>;
	if (NC == "fib")
		return code<NumbersCoder<fib>, SPEED>;

	return nullptr;
}

template <typename SPEED>
decode_function getDecodeFunction(std::string_view NC)
{
	if (NC == "gamma")
		return decode<NumbersCoder<E_gamma>, SPEED>;
	if (NC == "delta")
		return decode<NumbersCoder<E_delta>, SPEED>;
	if (NC == "omega")
		return decode<NumbersCoder<E_omega>, SPEED>;
	if (NC == "fib")
		return decode<NumbersCoder<fib>, SPEED>;

	return nullptr;
}

int main(int argc, char** argv)
{
	std::ios_base::sync_with_stdio(false);

	if (argc < 4)
	{
		std::cout << "Invalid arguments\n";
		return 0;
	}

	std::string input_file = argv[2];
	std::string output_file = argv[3];
	std::string numbers_coding = "omega";

	if (argc == 5)
	{
		numbers_coding = argv[4];
	}

	std::string job = argv[1];

	if (job == "code")
	{
		double entropy2{};
		bool success{ false };
		double entropy{}, avg_length{};
		double level{};
		std::uint64_t sizeC{};
		std::uint64_t sizeU{};

		//DictionaryHashTableHelper::fast faster encoding, great for big files, terrible for small
		//DictionaryHashTableHelper::slow slower encoding, terrible for big files, good for small
		auto coder = getCodeFunction<DictionaryHashTableHelper::slow>(numbers_coding);

		auto x = coder(input_file, output_file);

		if (!x.has_value())
		{
			success = false;
		}
		else
		{
			success = true;
			std::tie(sizeC, sizeU, entropy, entropy2, avg_length, level) = x.value();
		}

		if (!success)
		{
			std::cout << "Oh, no. Something went wrong\n";
			return 0;
		}

		std::cout << "Compressed file emtropy: " << entropy2 << "\n";
		std::cout << "Compressed file size: " << sizeC << "\n";
		std::cout << "Uncompressed file emtropy: " << entropy << "\n";
		std::cout << "Uncompressed file size: " << sizeU << "\n";
		std::cout << "Arg codeword length: " << avg_length << "\n";
		std::cout << "CR: " << level << "\n";
	}
	else if (job == "decode")
	{
		//DictionaryHashTableHelper::fast faster encoding, great for big files, terrible for small
		//DictionaryHashTableHelper::slow slower encoding, terrible for big files, good for small
		auto decoder = getDecodeFunction<DictionaryHashTableHelper::slow>(numbers_coding);

		auto x = decoder(input_file, output_file);

		if (!x)
		{
			std::cout << "Oh, no. Something went wrong\n";
			return 0;
		}
	}
	else
	{
		std::cout << "Invalid job\n";
		return 0;
	}

	return 0;
}


bool checkFiles(std::string_view input_file, std::string_view output_file)
{
	return input_file != output_file;
}

class FileIO
{
public:
	FileIO(std::string_view input_file, std::string_view output_file) {
		input.open(input_file.data(), std::ios_base::in | std::ios_base::binary);
		output.open(output_file.data(), std::ios_base::out | std::ios_base::binary);
	}
	FileIO(const FileIO&) = delete;
	FileIO(FileIO&&) = delete;
	FileIO& operator=(const FileIO&) = delete;
	FileIO& operator=(FileIO&&) = delete;

	~FileIO() {
		input.close();
		output.close();
	}

	bool isValid() const { return input && output; }

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

	template <typename T>
	void write(std::vector<T>& data) {
		output.write(reinterpret_cast<char*>(data.data()), data.size() * sizeof(T));
		data.clear();
	}

private:
	std::fstream input{};
	std::fstream output{};

	const std::size_t input_size = 100 * 1000 * 1024;
};

template <std::size_t N>
void copyToVector(std::bitset<N>& a, std::uint64_t& sh, std::vector<unsigned char>& b, std::vector<std::uint64_t>& characters_c_count)
{
	unsigned long tmp;
	std::vector<unsigned char> r_out{};

	while (sh >= 8)
	{
		tmp = a[0] | a[1] << 1 | a[2] << 2 | a[3] << 3 | a[4] << 4 | a[5] << 5 | a[6] << 6 | a[7] << 7;
		a >>= 8;
		sh -= 8;
		r_out.push_back(static_cast<unsigned char>(tmp & 0xFF));
		characters_c_count[r_out.back()]++;
	}

	std::copy(r_out.begin(), r_out.end(), std::back_inserter(b));
}

template <std::size_t N>
void copyToBitset(std::bitset<N>& a, std::uint64_t& sh, std::vector<unsigned char>::iterator& it)
{
	if (sh >= 8)
	{
		sh -= 8;
		a >>= 8;
		a[N - 1] = *it & 0x80;
		a[N - 2] = *it & 0x40;
		a[N - 3] = *it & 0x20;
		a[N - 4] = *it & 0x10;
		a[N - 5] = *it & 0x08;
		a[N - 6] = *it & 0x04;
		a[N - 7] = *it & 0x02;
		a[N - 8] = *it & 0x01;

	}

	++it;
}

template <std::size_t N>
void fillBitset(std::bitset<N>& a, std::uint64_t& sh, bool fill)
{
	if (sh >= 8)
	{
		sh -= 8;
		a >>= 8;
		a[N - 1] = fill << 7;
		a[N - 2] = fill << 6;
		a[N - 3] = fill << 5;
		a[N - 4] = fill << 4;
		a[N - 5] = fill << 3;
		a[N - 6] = fill << 2;
		a[N - 7] = fill << 1;
		a[N - 8] = fill;

	}
}

template <typename NC, typename SPEED>
std::optional<std::tuple<std::uint64_t, std::uint64_t, double, double, double, double>> code(std::string_view input_file, std::string_view output_file)
{
	if (!checkFiles(input_file, output_file))
	{
		return {};
	}

	FileIO file_IO{ input_file, output_file };

	if (!file_IO.isValid())
	{
		return {};
	}

	NC numbers_coding{};

	//Dictionary
	DictionaryHashTable<SPEED> DHT{ ProgramSettings::DHT_base_size };

	//Input buffer
	std::vector<unsigned char> in_buffer{};
	auto it = in_buffer.begin();

	//bitset shift
	std::uint64_t sh{};

	//Output buffer
	std::vector<unsigned char> out_b{};

	//Output buffer for number encoding
	std::bitset<512> out;

	constexpr std::size_t max_buffer_size = 100 * 1000 * 1024;

	//Entropy
	std::vector<std::uint64_t> characters_c_count{};
	std::vector<std::uint64_t> characters_u_count{};
	characters_u_count.resize(256);
	characters_c_count.resize(256);

	//hashed Index in dictionary
	std::uint64_t last_index = 0;
	std::uint64_t next_index = 0;

	//real index in dictionary
	std::size_t last_realID = 0;

	const DictionaryNode* node;

	std::uint64_t loaded{};
	std::uint64_t saved{};

	while (true)
	{
		//Load input buffer from file
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

		//get next index
		next_index = DHT.hash(last_index, *it);

		characters_u_count[*it]++;

		if ((node = DHT.at(next_index, last_realID)))
		{
			//Node exists, continue

			last_index = next_index;
			last_realID = DHT.getNodeRealID(node);
		}
		else
		{
			//Node don't exist, output last_realID
			//and add new node
			numbers_coding.encode(out, sh, last_realID);
			copyToVector<>(out, sh, out_b, characters_c_count);

			if (out_b.size() > max_buffer_size)
			{
				//dump output_buffer to file
				saved += out_b.size();
				file_IO.write(out_b);
			}

			if (DHT.size() < DHT.maxSize())
			{
				DHT.insert({ next_index, last_realID, *it });
			}

			if (DHT.size() >= DHT.maxSize())
			{
				//Dictionary is full
				//Clear current dictionary for new data
				DHT.clear();
			}

			//Initialize new string with base character (ASCII: 0-255)
			last_realID = DHT.getBaseNodeRealID(*it);
			last_index = DHT.at(last_realID)->index;
		}

		it++;
		loaded++;

		if (loaded % 1024000 == 0)
		{
			std::clog << loaded / 1024000 << " MB\n";
		}
	}

	numbers_coding.encode(out, sh, last_realID);

	//Align to 8 bit
	while (sh % 8 != 0)
	{
		out[sh] = numbers_coding.fill;
		sh++;
	}

	copyToVector<>(out, sh, out_b, characters_c_count);

	saved += out_b.size();
	file_IO.write(out_b);

	//Calculate Entropy
	std::uint64_t size_u = std::accumulate(characters_u_count.begin(), characters_u_count.end(), std::uint64_t{});
	std::uint64_t size_c = std::accumulate(characters_c_count.begin(), characters_c_count.end(), std::uint64_t{});
	double H_u_tmp = 1;
	double H_c_tmp = 1;
	for (auto&& x : characters_u_count)
	{
		if (x == 0) continue;

		H_u_tmp *= std::pow(x / (double)size_u, x / (double)size_u);

	}

	for (auto&& x : characters_c_count)
	{
		if (x == 0) continue;

		H_c_tmp *= std::pow(x / (double)size_c, x / (double)size_c);

	}

	double H_u = static_cast<double>(std::abs(std::log2l(H_u_tmp)));
	double H_c = static_cast<double>(std::abs(std::log2l(H_c_tmp)));

	return { { saved, loaded, H_u, H_c, (8.0 * saved) / (double)loaded, (double)loaded / (double)saved } };
}

template <typename NC, typename SPEED>
bool decode(std::string_view input_file, std::string_view output_file)
{
	if (!checkFiles(input_file, output_file))
	{
		return false;
	}

	FileIO file_IO{ input_file, output_file };

	if (!file_IO.isValid())
	{
		return false;
	}

	NC numbers_coding{};

	//Dictionary
	DictionaryHashTable<SPEED> DHT{ ProgramSettings::DHT_base_size };

	//Input buffer
	std::vector<unsigned char> in_b{};
	auto it = in_b.begin();

	//Number deencoder buffer
	std::bitset<512> in; //Important in_b.size() >= in.size()!!

	//Bitset shift
	std::uint64_t sh{ in.size() };

	//If all data is read, set it false
	bool continue_reading{ true };

	//Output buffer
	std::vector<unsigned char> out_buffer{};

	constexpr std::size_t max_buffer_size = 100 * 1000 * 1024;

	//hash index an real index
	std::size_t last_realID = 0;
	std::size_t last_index = 0;
	std::size_t next_realID = 0;
	std::size_t next_index = 0;

	auto tmp = next_realID;
	std::vector<unsigned char> to_append{};

	std::uint64_t loaded{ 0 };
	std::uint64_t load_level{ 1024000 * 8 };
	std::uint64_t saved{ 0 };

	//get data from file
	if (it == in_b.end())
	{
		file_IO.read(in_b);

		it = in_b.begin();
	}

	if (it == in_b.end())
	{
		return true;
	}

	while (sh > 0 && it != in_b.end())
	{
		copyToBitset(in, sh, it);
	}

	//Bitset always have to be full(sh < 8), so fill with fill bit
	while (sh > 0)
	{
		fillBitset(in, sh, numbers_coding.fill);
	}

	last_index = next_index;

	//Calculate new bitset shift
	auto sh_c = sh;
	//new real index to save
	last_realID = numbers_coding.decode(in, sh).first;
	loaded += sh - sh_c;

	tmp = last_realID;
	
	last_index = DHT.at(last_realID)->index;

	out_buffer.push_back(DHT.at(last_realID)->character);

	while (true)
	{
		//Write data to input buffer an bitset
		while (sh >= 8)
		{
			if (!continue_reading)
			{
				while (sh >= 8)
				{
					fillBitset(in, sh, numbers_coding.fill);
				}
			}
			else
			{
				while (sh >= 8 && it != in_b.end())
				{
					copyToBitset(in, sh, it);
				}

				if (it == in_b.end())
				{
					if (!file_IO.read(in_b))
					{
						break;
					}

					it = in_b.begin();

					if (it == in_b.end())
					{
						continue_reading = false;
					}
				}
			}
		}

		//Calculate new bitset shift
		sh_c = sh;
		//new real index to save
		auto next = numbers_coding.decode(in, sh);
		loaded += sh - sh_c;

		if (!next.second)
		{
			//Number encoder returned false, so there aren't any numbers in buffer
			break;
		}

		next_realID = next.first;

		if (DHT.at(next_realID)->index == 0)
		{
			//If coder returned unknow index, so it must be last string extended by first character
			//Append first character
			auto new_index = DHT.hash(last_index, DHT.at(tmp)->character);
			DHT.insert({ new_index, last_realID, DHT.at(tmp)->character });
		}

		//Read all character in list created by first node
		tmp = next_realID;
		to_append.clear();
		for (;;)
		{
			to_append.push_back(DHT.at(tmp)->character);

			if (!DHT.at(tmp)->parent)
				break;

			tmp = DHT.at(tmp)->parent;
		}
		//String is reversed, so copry with reverse iterrators
		std::copy(to_append.rbegin(), to_append.rend(), std::back_inserter(out_buffer));

		if (out_buffer.size() > max_buffer_size)
		{
			//Save output buffer to file
			saved += out_buffer.size();
			file_IO.write(out_buffer);
		}

		//Calcuate index for new string
		//Connect last string, with first character from next string
		next_index = DHT.hash(last_index, DHT.at(tmp)->character);

		if (DHT.size() < DHT.maxSize())
		{
			DHT.insert({ next_index, last_realID, DHT.at(tmp)->character });
		}

		if (DHT.size() >= DHT.maxSize())
		{
			//Dictionary is full
				//Clear current dictionary for new data
			DHT.clear();
		}

		last_realID = next_realID;
		last_index = DHT.at(last_realID)->index;
		

		if (loaded >= load_level)
		{
			std::clog << loaded / (1024000 * 8) << " MB\n";
			load_level += 1024000 * 8;
		}
	}

	saved += out_buffer.size();
	file_IO.write(out_buffer);

	return true;
}
