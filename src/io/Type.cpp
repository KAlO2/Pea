#include "io/Type.h"

#include "graphics/Image.h"
#include "util/Log.h"

#include <cassert>
#include <cinttypes>
#include <stdio.h>
#include <string.h>

using namespace pea;

static const char* TAG = "Primitive";

int32_t Type::parseInt(const char* &token)
{
	token += strspn(token, " \t");
	int i = atoi(token);
	token += strcspn(token, " \t\r");
	return i;
}

uint32_t Type::parseUint(const char* &token)
{
	char* end;
	uint32_t number = std::strtoul(token, &end, 10);
	
	if(errno != 0)
		token = end;
	else
	{
		if(errno == ERANGE)
			slog.w(TAG, "range error, expect %.*s, got %" PRIu32, static_cast<int32_t>(end - token), token, number);
		else
			slog.w(TAG, "errno=%d", errno);
		errno = 0;
	}
	
	return number;
}

float Type::parseFloat(const char* &token)
{
	token += strspn(token, " \t");
	float f = static_cast<float>(atof(token));
	token += strcspn(token, " \t\r");
	return f;
}

vec3i Type::parseInt3(const char* &token)
{
	vec3i index(0);
	index.i = std::atoi(token);
	token += strcspn(token, " \t\r/");
	if(*token != '/') // i
		return index;
	++token;

	if(*token == '/')  // i//k
	{
INDEX_K:
		++token;
		index.k = std::atoi(token);
		token += strcspn(token, " \t\r/");
		return index;
	}

	index.j = std::atoi(token);
	token += strcspn(token, " \t\r/");
	if(*token == '/')  // i/j/k
		goto INDEX_K;

	// i/j
	token += strcspn(token, " \t\r/");

	return index;
/*
	int32_t i = 0, j = 0, k = 0;  // invalid index

	std::istringstream tuple(token);
	tuple >> i;
	assert(!tuple.fail());

	int d1 = tuple.get();
	if(!tuple.eof())
	{
		assert(d1 == '/');
		if(tuple.peek() != '/')
		{
			tuple >> j;  // i/j

			const int d2 = tuple.get();
			if(!tuple.eof())  // i/j/k
			{
				assert(d2 == '/');
				tuple >> k;
			}
		}
		else  // i//k
		{
			tuple.get();  // remove the following delimiter
			tuple >> k;
		}
	}

//	--i; --j; --k;  // .OBJ count from 1, but C count from 0
	return vec3i(i, j, k);
*/
}

vec3u Type::parseUint3(const char* &token)
{
	vec3i triple = parseInt3(token);
#ifndef NDEBUG
	if(triple[0] < 0 || triple[0] < 0 || triple[0] < 0)
		slog.w(TAG, "triple index %" PRIu32 "/%" PRIu32 "/%" PRIu32
				" is out of range", triple[0], triple[1], triple[2]);
#endif
	return vec3u(triple[0], triple[1], triple[2]);
}

vec2f Type::parseFloat2(const char* &token)
{
	float x = parseFloat(token);
	float y = parseFloat(token);
	return vec2f(x, y);
}

vec3f Type::parseFloat3(const char* &token)
{
	float x = parseFloat(token);
	float y = parseFloat(token);
	float z = parseFloat(token);
	return vec3f(x, y, z);
}

vec4f Type::parseFloat4(const char* &token)
{
	float x = parseFloat(token);
	float y = parseFloat(token);
	float z = parseFloat(token);
	float w = parseFloat(token);
	return vec4f(x, y, z, w);
}

std::string Type::parseString(const char* &token)
{
	size_t length = strcspn(token, " \t");
#ifndef NDEBUG
	const char* start = token + length;
	const char* end = start + strspn(start, " \t");
	if(*end != '\0')
		slog.w(TAG, "redundant content \"%s\"", start);
#endif
	return std::string(token, length);
}

std::vector<std::string> Type::split(const char* str, const char* delim)
{
	assert(str != nullptr && delim != nullptr);
	std::vector<std::string> tokens;
	const char* start = str + strspn(str, delim);
	while(*start)
	{
		size_t length = strcspn(start, delim);
		tokens.push_back(std::string(start, length));
		start += length;
		start += strspn(start, delim);
	}
	return tokens;
}

std::vector<std::string> Type::split(const char* str, char delim)
{
	assert(str != nullptr);
	std::vector<std::string> result;

	const char* p = str;
	while(true)
	{
		for(; *p != '\0' && *p == delim; ++p);
		if(*p == '\0')
			break;
		else
			str = p;

		for(; *p != '\0' && *p != delim; ++p);
		if(p != str)
			result.push_back(std::string(str, p - str));
	}

	return result;
}

std::vector<std::string> Type::split(const char* str)
{
	std::vector<std::string> names;
	const char* p = str;
	while(true)
	{
		const char* end = p;
		while(isalnum(*end))
			++end;
		if(p != end)  // [p, end), half open, half close.
			names.push_back(std::string(p, end));

		p = end;
		while(!isalnum(*p) && *p != '\0')
			++p;

		if(*p == '\0')
			break;
	}

	return names;
}
