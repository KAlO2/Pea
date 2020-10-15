#ifndef PEA_OPENGL_UNIFORM_BLOCK_H_
#define PEA_OPENGL_UNIFORM_BLOCK_H_

#include <cinttypes>
#include <vector>

#include "io/Type.h"

namespace pea {

struct Uniform
{
	int32_t location;
	int32_t offset;
	Type type;
};

class UniformBlock
{
private:
	std::vector<Uniform> uniforms;
	// TODO: make memory data 4 bytes aligned?
	std::vector<unsigned char> uniformData;
	
public:
	void appendUniform(int32_t location, Type type, void* data);
	
	bool hasUniform(int32_t location) const;
	bool hasUniform(int32_t location, Uniform* &uniform);
	
	void updateUniform(int32_t location, Type type, void* data);
	
	void updateUniform(Uniform& uniform, void* data);
	
	void feedUniforms() const;
};

}  // namespace pea
#endif  // PEA_OPENGL_UNIFORM_BLOCK_H_
