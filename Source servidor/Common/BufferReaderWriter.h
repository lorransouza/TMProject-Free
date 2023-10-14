#pragma once

#include <vector>
#include <type_traits>
#include <string>
#include <sstream>


class BufferReader
{
private:
	std::vector<unsigned char> data;
	int index = 0;

public:
	BufferReader(unsigned char* input, int size)
	{
		data.resize(size);
		memcpy(data.data(), input, size);
	}

	void advance(unsigned int size)
	{
		index += size;
	}

	template<typename T>
	T Get()
	{
		T val = *(T*)&data[index];

		index += sizeof(T);
		return val;
	}

	template<>
	st_Item Get()
	{
		st_Item item{};
		memcpy(&item, &data[index], sizeof st_Item);

		index += 8;
		return item;
	}

	BufferReader& operator+=(unsigned int value)
	{
		index += value;

		return *this;
	}

	template<>
	std::string Get()
	{
		auto size = Get<unsigned int>();
		if (size + index > data.size())
		{
			std::stringstream str;
			str << "Invalid data. String size " << size << std::endl;
			str << "Actual index " << index << std::endl;
			str << "Data size: " << data.size();

			return str.str();
		}

		std::vector<unsigned char> temporaryData(size);

		memcpy_s(temporaryData.data(), size, &data[index], size);

		std::string val(temporaryData.begin(), temporaryData.end());
		index += size;
		return val;
	}

	template<typename T>
	T* GetAs()
	{
		return reinterpret_cast<T*>(data.data());
	}
};

class BufferWriter
{
	size_t index{ 0 };
	std::vector<unsigned char> data;
public:
	BufferWriter(int size)
	{
		data.resize(size);
	}

	BufferWriter() = default;

	void advance(unsigned int size)
	{
		index += size;
	}

	BufferWriter& operator+=(unsigned int value)
	{
		index += value;

		return *this;
	}

	template<typename T>
	void Set(T value, int position = -1)
	{
		bool moveIndex{ false };
		if (position == -1)
		{
			position = index;
			moveIndex = true;
		}

		if (position + sizeof(T) > data.size())
			data.resize(position + sizeof(T));

		if constexpr (std::is_class<T>::value)
			memcpy(&data[position], (void*)&value, sizeof(T));
		else
			*reinterpret_cast<T*>(&data[position]) = value;

		if (moveIndex)
			index += sizeof(T);
	}

	template<typename T = std::string>
	void Set(const char* value, size_t size)
	{
		Set<unsigned int>(size);

		if (index + size > data.size())
			data.resize(index + size);

		memcpy_s(&data[index], data.size() - index, value, size);

		index += size;
	}

	std::vector<unsigned char> GetBuffer()
	{
		return data;
	}

	template<typename T>
	T* GetAs()
	{
		return reinterpret_cast<T*>(data.data());
	}
};
