#pragma once

#include <cstdint>
#include <utility>
#include <memory>

struct DictionaryNode
{
	std::size_t index{};
	std::size_t parent{};
	unsigned char character{};
};

namespace DictionaryHashTableHelper
{
	using slow = std::integral_constant<int, 1>;
	using fast = std::integral_constant<int, 2>;
}

template<typename SPEED = DictionaryHashTableHelper::slow>
class DictionaryHashTable
{
public:
	DictionaryHashTable(std::size_t base_size);

	std::size_t size() const
	{
		return _size;
	}

	std::size_t maxSize() const {
		if constexpr (std::is_same_v<SPEED, DictionaryHashTableHelper::slow>)
		{
			return base_size - 1;
		}
		else
		{
			//Trying full whole dictionary is very slow, so leave some free space for speed
			return static_cast<std::size_t>(base_size * 0.8);
		}
	}

	void clear();

	std::size_t insert(const DictionaryNode& node);

	const DictionaryNode* at(std::uint64_t index, std::uint64_t parent) const;
	const DictionaryNode* at(std::size_t realID) const;
	std::size_t getNodeRealID(const DictionaryNode& node) const;
	std::size_t getNodeRealID(const DictionaryNode* node) const
	{
		//RealID is just array index
		return node - array.get();
	}

	std::size_t getBaseNodeRealID(unsigned char x)
	{
		//RealID of standard ASCII character
		auto index = hash(0, x);
		return getNodeRealID({ index, 0, x });
	}

	std::uint64_t hash(std::uint64_t a, std::uint64_t b)
	{
		if constexpr (std::is_same_v<SPEED, DictionaryHashTableHelper::slow>)
		{
			//Very compressed dictionary, but very slow
			if (a > std::numeric_limits<std::uint64_t>::max() - 1 - b)
			{
				return a - (std::numeric_limits<std::uint64_t>::max() - 1 - b);
			}

			return a + b + 1;
		}
		else
		{
			//Dictionary this hash generate better randomnes, but will not compress small files
			//Only works in big dictionaries
			//Best if input_size >> dictionary_size
			auto seed = std::hash<std::uint64_t>{}(a);

			seed ^= std::hash<std::uint64_t>{}(b) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			return seed;
		}
	}

private:
	std::size_t toIndex(std::uint64_t val) const
	{
		//Calculate index from hash
		auto x = val % (base_size);

		return x == 0 ? (base_size - 1) : x;
	}

	const std::size_t base_size;
	std::unique_ptr<DictionaryNode[]> array;
	std::size_t _size{};
};

template<typename SPEED>
DictionaryHashTable<SPEED>::DictionaryHashTable(std::size_t base_size) : base_size{ base_size }, array{ new DictionaryNode[base_size]{} }
{
	DictionaryNode node{};

	//Set up ASCII characters
	for (int i = 0; i <= 255; i++)
	{
		node.character = static_cast<unsigned char>(i);
		node.index = DictionaryHashTable::hash(0, i);

		insert(node);
	}
}

template<typename SPEED>
void DictionaryHashTable<SPEED>::clear()
{
	_size = 0;

	array.reset(new DictionaryNode[base_size]{});

	DictionaryNode node{};

	for (int i = 0; i <= 255; i++)
	{
		node.character = static_cast<unsigned char>(i);
		node.index = DictionaryHashTable::hash(0, i);

		insert(node);
	}
}

template<typename SPEED>
std::size_t DictionaryHashTable<SPEED>::insert(const DictionaryNode& node)
{
	//Find first node with index == 0 and realID >= toIndex(node.index)
	//If node already exists return it position
	auto by_index = &array[toIndex(node.index)];

	if (by_index->index == node.index && by_index->parent == node.parent)
		return by_index - array.get();

	if (by_index->index == 0)
	{
		by_index->index = node.index;
		by_index->parent = node.parent;
		by_index->character = node.character;
		_size++;

		return by_index - array.get();
	}

	auto end = array.get() + base_size;
	auto ptr = by_index + 1;

	while (ptr != by_index)
	{
		if (ptr == end)
		{
			//Connect end with begin
			ptr = array.get() + 1;
		}

		if (ptr->index == node.index && ptr->parent == node.parent)
			return ptr - array.get();

		if (ptr->index == 0)
		{
			ptr->index = node.index;
			ptr->parent = node.parent;
			ptr->character = node.character;
			_size++;

			return ptr - array.get();
		}

		ptr++;
	}

	return 0;
}

template<typename SPEED>
const DictionaryNode* DictionaryHashTable<SPEED>::at(std::uint64_t index, std::uint64_t parent) const
{
	//Find first node with node.index == index and node.parent = parent and realID >= toIndex(node.index)
	auto by_index = &array[toIndex(index)];

	if (by_index->index == index && by_index->parent == parent)
		return by_index;

	auto end = array.get() + base_size;
	auto ptr = by_index + 1;

	while (ptr != by_index)
	{
		if (ptr == end)
		{
			//Connect end with begin
			ptr = array.get() + 1;
		}

		if (ptr->index == 0)
			return nullptr;

		if (ptr->index == index && ptr->parent == parent)
			return ptr;

		ptr++;
	}

	return nullptr;
}

template<typename SPEED>
const DictionaryNode* DictionaryHashTable<SPEED>::at(size_t realID) const
{
	return &array[realID];
}

template<typename SPEED>
std::size_t DictionaryHashTable<SPEED>::getNodeRealID(const DictionaryNode& node) const
{
	//Just DictionaryHashTable<SPEED>::at, but return realID
	auto by_index = &array[toIndex(node.index)];

	if (by_index->index == node.index && by_index->parent == node.parent && by_index->character == node.character)
	{
		return by_index - array.get();
	}

	auto end = array.get() + base_size;
	auto ptr = by_index + 1;

	while (ptr != by_index)
	{
		if (ptr == end)
		{
			ptr = array.get() + 1;
		}

		if (ptr->index == node.index && ptr->parent == node.parent && ptr->character == node.character)
		{
			return by_index - array.get();
		}

		ptr++;
	}

	return 0;
}
