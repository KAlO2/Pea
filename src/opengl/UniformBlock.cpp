#include "opengl/UniformBlock.h"

#include <cassert>
#include <cstring>

#include "opengl/Program.h"

using namespace pea;

void UniformBlock::appendUniform(int32_t location, Type type, void* data)
{
	assert(!hasUniform(location));
	
//	uint32_t alignedSize = (typeSize + 3) & ~4;
	uint32_t typeSize = sizeofType(type);
	const size_t size = uniformData.size();
	uniformData.resize(size + typeSize);
	std::memcpy(uniformData.data() + size, data, typeSize);
	
	Uniform uniform;
	uniform.location = location;
	uniform.type = type;
	uniform.offset = size;
	uniforms.push_back(uniform);
}

bool UniformBlock::hasUniform(int32_t location) const
{
	for(const Uniform& uniform: uniforms)
		if(uniform.location == location)
			return true;
	return false;
}

bool UniformBlock::hasUniform(int32_t location, Uniform* &uniform)
{
	for(Uniform& uniform_: uniforms)
	{
		if(uniform_.location == location)
		{
			if(uniform)
				uniform = &uniform_;
			return true;
		}
	}
	
	return false;
}

void UniformBlock::updateUniform(int32_t location, Type type, void* data)
{
	assert(hasUniform(location));
	
	uint32_t typeSize = sizeofType(type);
	for(const Uniform& uniform: uniforms)
	{
		// uniform exists, update its value
		if(uniform.location == location)
		{
			std::memcpy(uniformData.data() + uniform.offset, data, typeSize);
			break;
		}
	}
}

void UniformBlock::updateUniform(Uniform& uniform, void* data)
{
	assert(hasUniform(uniform.location));
	assert(0 <= uniform.offset && uniform.offset < static_cast<int32_t>(uniformData.size()));
	uint32_t typeSize = sizeofType(uniform.type);
	std::memcpy(uniformData.data() + uniform.offset, data, typeSize);
}

void UniformBlock::feedUniforms() const
{
	for(const Uniform& uniform: uniforms)
	{
		const unsigned char* data = uniformData.data() + uniform.offset;
		switch(uniform.type)
		{
#define CASE(TYPE, T) \
		case Type::TYPE: Program::setUniform(uniform.location, *reinterpret_cast<const T*>(data)); break;
		
		CASE(INT32, int32_t)
		CASE(UINT32, uint32_t)
		CASE(FLOAT, float)
		CASE(VEC2F, vec2f)
		CASE(VEC3F, vec3f)
		CASE(VEC4F, vec4f)
#undef CASE
		default:
			assert(false);
			break;
		}
	}
}

