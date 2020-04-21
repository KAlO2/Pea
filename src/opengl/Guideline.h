#ifndef PEA_OPENGL_GUIDELINE_H_
#define PEA_OPENGL_GUIDELINE_H_

#include "math/Transform.h"
#include "scene/Object.h"

namespace pea {

class Guideline: private Object
{
public:
	static constexpr uint8_t WORLD_SHIFT = 1;
	static constexpr uint8_t LOCAL_SHIFT = 4;
	enum Flag: std::uint32_t
	{
		GRID    = 1,
		
		WORLD_X = Transform::X << WORLD_SHIFT,
		WORLD_Y = Transform::Y << WORLD_SHIFT,
		WORLD_Z = Transform::Z << WORLD_SHIFT,
		
		LOCAL_X = Transform::X << LOCAL_SHIFT,
		LOCAL_Y = Transform::Y << LOCAL_SHIFT,
		LOCAL_Z = Transform::Z << LOCAL_SHIFT,
	};
	
private:
	vec4f gridColor;
	
	uint32_t vao;
	uint32_t vbo;
	Flag     flag;
	
public:
	Guideline();
	~Guideline();
	
	void prepare();
	
	void uploadVertexData() const;
	
	/**
	 * @param[in] flag LOCAL_X, LOCAL_Y, LOCAL_Z, or their combinations
	 * @param[in] positions vertex positions. 2 vertices for LOCAL_X,. 4 vertices for 
	 *            LOCAL_X | LOCAL_Y, 6 vertices for LOCAL_X | LOCAL_Z.
	 */
	void updateVertexData(Flag flag, const vec3f* positions);
	
	void setFlag(Flag flag);
	Flag getFlag() const;
	
	void addFlag(Flag flag);
	void clearFlag(Flag flag);
	
	void  setGridColor(const vec4f& color);
	vec4f getGridColor() const;
	
	void render(const mat4f& viewProjection) const override;

};

inline void  Guideline::setGridColor(const vec4f& color) { gridColor = color; }
inline vec4f Guideline::getGridColor() const             { return gridColor;  }

}  // namespace pea
#endif  // PEA_OPENGL_GUIDELINE_H_
