#pragma once
#include <stdexcept>

template <class T>
class CircularBuffer
{
public:
	CircularBuffer(unsigned int _max_size)
	{
		if (_max_size == 0)
		{
			throw std::out_of_range("can't have a 0 sized buffer");
		}
		max_size = _max_size;
		buffer = new T[_max_size];
		memset(buffer, 0, sizeof(T)*max_size);
	}

	~CircularBuffer()
	{
		if (buffer)
		{
			delete[] buffer;
		}
	}

	void clear()
	{
		unsigned int start_index = 0;
		unsigned int end_index = 0;
		memset(buffer, 0, sizeof(T)*max_size);
	}

	T get(unsigned int index)
	{
		unsigned int actual_index = (index + start_index) % max_size;
		return buffer[actual_index];
	}

	void push(T value)
	{
		buffer[end_index] = value;
		end_index++;
		
		if (end_index >= max_size)
		{
			end_index = 0;
		}

		// shift the start index along once the end index wraps round to be in front of it
		if (end_index <= start_index)
		{
			start_index++;
		}

		if (start_index >= max_size)
		{
			start_index = 0;
		}
	}

private:
	T* buffer = nullptr;
	unsigned int max_size = 0;
	unsigned int start_index = 0;
	unsigned int end_index = 0;
};