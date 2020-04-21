#ifndef PEA_MATH_RANDOM_H_
#define PEA_MATH_RANDOM_H_

#include "math/vec2.h"
#include "math/vec3.h"

namespace pea {

/**
 * random number generator
 */
class Random
{
private:
//	static thread_local std::mt19937 generator;
	
public:
	Random() = default;
	~Random() = default;
	
	static void setSeed(uint32_t seed);
	
	static float emit();
	static float emit(float min, float max);
	
	static vec3f sphereEmit(float radius);
	
	static vec2f diskEmit(float radius);
	
	static vec2f circleEmit(float radius);
	
};

}  // namespace pea
#endif  // PEA_MATH_RANDOM_H_
