#pragma once
#include "SerialInterface.h"

struct Variant
{
	union
	{
		uint32_t as_int;
		double as_double;
		char* as_string;
	};

	enum Type
	{
		TYPE_INTEGER,
		TYPE_DOUBLE,
		TYPE_STRING,
		TYPE_INVALID
	} type;
	
	Variant() {}
	~Variant() {}
};

Variant parse_message(Message& m, const char field)
{
    const char* message = m.data.c_str();
	Variant result;
	size_t length = strlen(message);
	// Check if message is non-empty
	if (strlen(message) == 0)
	{
		result.type = Variant::TYPE_INVALID;
		return result;
	}

	// Find the offset of the requested field (if it exists)
	// If it doesn't, offset will be set to -1 indicating a fail
	int offset = 0;
	while (message[offset] != field)
	{
		// Find the end of the current data field
		uint32_t i = offset;
		while (message[i] != ' ' && message[i] != '\0')
		{
			i++;
		}
		// If there are no more, `i` will still be equal to the offset so indicate
		// that the requested field could not be found. 
		// Or if this is the end of the string
		if (i == offset || message[++i] == '\0')
		{
			offset = -1;


			break;
		}

		offset = i;
	}

	// If the offset is not the end of the string or -1, get the value from it
	if (offset != -1)
	{
		// Now that the correct start offset was found, find the end of the field
		// i.e. the position of the next ' ' or '\0'
		uint32_t end_pos = offset;
		while (message[end_pos] != ' ' && message[end_pos] != '\0')
		{
			++end_pos;
		}
		// If field is empty, return an invalid type
		if (end_pos == offset + 1)
		{
			result.type = Variant::TYPE_INVALID;
			return result;
		}

		// Figure out if we're looking at an integer or a floating point number
		bool is_double = false;
		for (uint32_t i = offset; i < end_pos; ++i)
		{
			if (message[i] == '.')
			{
				is_double = true;
				break;
			}
		}
		
		if (is_double)
		{
			result.type = Variant::TYPE_DOUBLE;
			result.as_double = strtod(&message[offset + 1], nullptr);
		}
		else
		{
			result.type = Variant::TYPE_INTEGER;
			result.as_int = strtol(&message[offset + 1], nullptr, 10);
		}
	}
	else // The string doesn't contain the field
	{
		result.type = Variant::TYPE_INVALID;
	}

	return result;
}