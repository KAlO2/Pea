#ifndef PEA_OPENGL_QUAD_RENDERER_H_
#define PEA_OPENGL_QUAD_RENDERER_H_

#include "opengl/Texture.h"
#include "scene/Object.h"

namespace pea {

class QuadRenderer: public Object
{
private:
	uint32_t vao;
	uint32_t vbo;
	const Texture* texture;
	
public:
	QuadRenderer();
	~QuadRenderer();
	
	void prepare();
	
	void upload() const;
	
	void setProgram(uint32_t program);
	
	void updateTexcoord(const vec2f quad[4]);
	
	/**
	 * @param[in] texture
	 */
	void setTexture(const Texture& texture);
	
	/**
	 * @param[in] viewProjection unused parameter
	 */
	void render(const mat4f& viewProjection) const override;
};

}  // namespace pea
#endif  // PEA_OPENGL_QUAD_RENDERER_H_
