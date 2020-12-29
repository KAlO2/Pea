#ifndef PEA_IO_TYPE_H_
#define PEA_IO_TYPE_H_

#include <cinttypes>

namespace pea {


enum class Type: unsigned char
{
	UNKNOWN = 0,

	BOOL,
	
	INT8,
	INT16,
	INT32,
	INT64,
	
	UINT8,
	UINT16,
	UINT32,
	UINT64,
	
	HALF,
	FLOAT,
	DOUBLE,

	VEC2I,
	VEC3I,
	VEC4I,

	VEC2U,
	VEC3U,
	VEC4U,
	
	VEC2F,
	VEC3F,
	VEC4F,

	MAT2F,
	MAT3F,
	MAT4F,
	
	STRING,
};

std::uint32_t sizeofType(Type type);


}  // namespace pea
#endif  // PEA_IO_TYPE_H_
