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
	for(const Uniform& uniform: uniforms)
	{
		if(uniform.location == location)  // Uniform exists, update its value.
		{
			assert(uniform.type == type);
			uint32_t typeSize = sizeofType(uniform.type);
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

void UniformBlock::removeUniform(int32_t location)
{
/*
	auto isSameLocation = [location] (const Uniform& uniform) {uniform.location = location;};
	auto it = std::find(uniforms.begin(), uniforms.end(), isSameLocation);
	if(it == uniforms.end())
		return;
*/
	const size_t count = uniforms.size();
	int32_t index = 0;
	for(; index < count; ++index)
		if(uniforms[index].location == location)
			break;
	
	if(index >= count)  // if not found
		return;
	
	int32_t size = sizeofType(uniforms[index].type);
	int32_t offset = uniforms[index].offset;
	for(index += 1; index < count; ++index)
	{
		assert(uniforms[index].offset > offset);
		uniforms[index - 1] = uniforms[index];
		unsigned char* data = uniformData.data() + uniforms[index].offset;
		std::memmove(data - size, data, size);
	}
	uniformData.resize(uniformData.size() - size);
}

void UniformBlock::removeUniforms()
{
	uniforms.clear();
	uniformData.clear();
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

std::string UniformBlock::toString() const
{
	std::ostringstream oss;
	for(const Uniform& uniform: uniforms)
	{
		oss << "location=" << uniform.location << ", value=";
		const unsigned char* data = uniformData.data() + uniform.offset;
		switch(uniform.type)
		{
#define CASE(TYPE, T) \
		case Type::TYPE: oss << *reinterpret_cast<const T*>(data); break;
		
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
		oss << '\n';
	}
	return oss.str();
}
