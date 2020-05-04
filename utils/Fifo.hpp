#pragma once
#include <stdexcept>

template <class T>
class Fifo
{
public:
	Fifo(unsigned int _max_size)
	{
		max_size = _max_size;
		buffer = new T[max_size];
	}

	~Fifo()
	{
		delete[] buffer;
	}

	T peek(unsigned int offset = 0)
	{
		unsigned int index = top_index + offset;

		if (index >= max_size)
		{
			throw std::out_of_range("Offset out of bounds");
		}

		return buffer[index];
	}

	void push(T value)
	{
		if (current_size == max_size)
		{
			throw std::out_of_range("Fifo is full");
		}

		if (top_index != 0)
		{
			rearrange();
		}

		buffer[bottom_index] = value;
		bottom_index++;
		current_size++;
	}

	T pop()
	{
		if (current_size == 0)
		{
			throw std::out_of_range("Fifo is empty");
		}

		T value = buffer[top_index];
		top_index++;
		current_size--;
		return value;
	}

	void clear()
	{
		memset((unsigned char*)buffer, 0, max_size * sizeof(T));

		current_size = 0;
		top_index = 0;
		bottom_index = 0;
	}

	bool is_empty()
	{
		return current_size == 0;
	}

	bool is_full()
	{
		return current_size == max_size;
	}

	unsigned int get_current_size()
	{
		return current_size;
	}

	unsigned int get_max_size()
	{
		return max_size;
	}

	void rearrange()
	{
		unsigned int copy_buffer_size = current_size;
		T* copy_buffer = new T[copy_buffer_size];
		memcpy((unsigned char*)copy_buffer, (unsigned char*)buffer + top_index, copy_buffer_size*sizeof(T));
		clear();
		memcpy((unsigned char*)buffer, (unsigned char*)copy_buffer, copy_buffer_size*sizeof(T));

		current_size = copy_buffer_size;
		bottom_index = current_size;

		delete copy_buffer;
	}

private:
	T* buffer = nullptr;
	unsigned int max_size = 0;
	unsigned int current_size = 0;
	unsigned int top_index = 0;
	unsigned int bottom_index = 0;
};