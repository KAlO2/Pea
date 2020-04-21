#ifndef PEA_SCENE_OBJECT_H_
#define PEA_SCENE_OBJECT_H_

#include "math/mat4.h"

namespace pea {

class Object
{
private:
	mat4f transform;
	uint32_t program;
	bool light;
public:
	Object();
	
	// Abstract base class needs a virtual destructor.
	virtual ~Object() = default;
	
	/**
	 * @param[in] transform The model matrix
	 */
	void setTransform(const mat4f& transform);
	
	const mat4f& getTransform() const;
	
	/**
	 * This doesn't hold program's ownership.
	 * @param[in] program a valid program object.
	 */
	void setProgram(uint32_t program);
	
	const uint32_t& getProgram() const;

	void illuminate(bool light);
	bool isIlluminated() const;
	
	/**
	 * This function also gets called once per frame, after the
	 * Renderable::update() call. Anything you do in this function should be
	 * strictly related to drawing something on the screen. Moving it,
	 * changing its appearance/properties/etc should happen in update().
	 */
	virtual void render(const mat4f& viewProjection) const = 0;
	
	/**
	 * This function gets called once per frame. Any game logic for an Actor
	 *  should be done in this function, since it provides you with a dt for
	 *  controlling rate of movement, animation, etc.
	 *
	 * @param dt The amount of time (in seconds) that has elapsed since the
	 *  last frame.
	 */
	virtual void update(float dt);


};

inline void Object::setTransform(const mat4f& transform) { this->transform = transform; }
inline const mat4f& Object::getTransform() const { return transform; }

inline const uint32_t& Object::getProgram() const { return program; }

inline void Object::illuminate(bool light) { this->light = light; }
inline bool Object::isIlluminated() const  { return light;        }

}  // namespace pea
#endif  // PEA_SCENE_OBJECT_H_
