#ifndef PEA_MATH_TRANSFORM_H_
#define PEA_MATH_TRANSFORM_H_

#include "math/vec3.h"
#include "math/mat4.h"

namespace pea {

enum class RotationOrder: uint8_t
{
	XYZ,
	XZY,
	YXZ,
	YZX,
	ZXY,
	ZYX,  // default
};

class alignas(4) Transform
{
public:
	static constexpr uint8_t GLOBAL = 0;
	static constexpr uint8_t LOCAL = 1;
	
	static constexpr uint8_t X = 0b001;
	static constexpr uint8_t Y = 0b010;
	static constexpr uint8_t Z = 0b100;
	static constexpr uint8_t AXIS_MASK = X | Y | Z;
	
	static constexpr uint8_t T = 0b100;
	static constexpr uint8_t R = 0b010;
	static constexpr uint8_t S = 0b001;
	
	static constexpr uint16_t TRANSLATION_SHIFT = 6;
	static constexpr uint16_t ROTATION_SHIFT    = 3;
	static constexpr uint16_t SCALING_SHIFT     = 0;
	
	static constexpr uint16_t TRANSLATION_MASK = 0b111'000'000;
	static constexpr uint16_t ROTATION_MASK    = 0b000'111'000;
	static constexpr uint16_t SCALING_MASK     = 0b000'000'111;
	

public:
	vec3f translation;
	vec3f rotation;  // angle in radians
	vec3f scaling;

	uint16_t axisLock;
	RotationOrder rotationOrder;
	
public:
	Transform();
	
	/**
	 * @param[in] transform It must be decomposable to TRS. This implies that transformation 
	 * matrices cannot skew or shear. TRS properties are converted to matrices and postmultiplied in
	 * the T * R * S order to compose the transformation matrix; first the scale is applied to the 
	 * vertices, then the rotation, and then the translation.
	 */
	void setTransform(const mat4f& transform);
	
	mat4f getTransform() const;
	mat4f getInverseTransform() const;
	
	void setTransform(uint8_t axis, const vec3f& variable);
	const vec3f& getTransform(uint8_t axis) const;
	
//	Transform& inverse();
	
	/**
	 * @param[in] axis X, Y, Z or combinations.
	 */
	void translate(uint8_t axis, float offset);
	
	/**
	 * @param[in] axis X, Y, Z or combinations. value X or Y | Z will rotate around X axis, 
	              X | Y | Z is not permitted.
	 */
	void rotate(uint8_t axis, float angle);
	void scale(uint8_t axis, float factor);
	
	void reset();
	
	void lock(uint16_t mask, uint8_t axis);
	void unlock(uint16_t mask, uint8_t axis);
	bool isLocked() const;
};

}  // namespace pea
#endif  // PEA_MATH_TRANSFORM_H_
