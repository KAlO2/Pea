#include "io/Type.h"

namespace pea {

std::uint32_t sizeofType(Type type)
{
	switch(type)
	{
	case Type::BOOL:
	case Type::INT8:
	case Type::UINT8:
		return 1;
		
	case Type::INT16:
	case Type::UINT16:
	case Type::HALF:
		return 2;
	
	case Type::INT32:
	case Type::UINT32:
	case Type::FLOAT:
		return 4;
	
	case Type::INT64:
	case Type::UINT64:
	case Type::DOUBLE:
		return 8;
	
	case Type::VEC2I:
	case Type::VEC2F:
		return 8;
	
	case Type::VEC3I:
	case Type::VEC3F:
		return 12;
	
	case Type::VEC4I:
	case Type::VEC4F:
		return 16;
	
	}
}

}  // namespace pea
