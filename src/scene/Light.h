#ifndef PEA_SCENE_LIGHT_H_
#define PEA_SCENE_LIGHT_H_

#include <string>

#include "math/vec3.h"
#include "math/vec4.h"

namespace pea {

/**
 * @class Light
 * Genesis 1:3 And God said, let there be light: and there was light.
 */
class Light
{
public:
	enum class Type: std::uint8_t
	{
		/**
		 * A point light source has a well-defined position with no direction. Because it emits light
		 * in all directions. A normal bulb is a point light.
		 */
		POINT,

		/**
		 * A spot light source emits light in a specific angle, and it has position and direction
		 * attributes. A good example for a spot light is a light spot in sport arenas.
		 */
		SPOT,

		/**
		 * A directional light is a light coming from a certain direction in world space, so it has
		 * no specific position in the scene, a good approximation can be sun light or moon light.
		 */
		DIRECTIONAL,
	};

private:
	// keep light intact with OpenGL functions.
	const Type type;
	
protected:

//	bool enabled;
	Light();
	explicit Light(Type type);
	~Light() = default;
public:
	vec3f ambient;
	vec3f diffuse;
	vec3f specular;
	
public:

	const Type& getType() const;
//	void setEnabled(bool enabled) { this->enabled = enabled; }
//	bool isEnabled() const { return enabled; }
};

inline const Light::Type& Light::getType() const { return type; }

class PointLight: public Light
{
public:
	static const char* SOURCE;
	
public:
	
	vec3f position;
	vec3f attenuation;  ///< attenuation factors (constant, linear, quadratic)
	float range;        ///< everything within this range will be lighted.
protected:
	PointLight(Type type, const vec3f& position);
public:
	explicit PointLight(const vec3f& position);
//	Light::Type getLightType() const override { return Light::Type::POINT; }

};

class DirectionalLight: public Light
{
private:
	
public:
	static const char* SOURCE;
	
	vec3f direction;
	
	explicit DirectionalLight(const vec3f& direction);
	
	
//	Light::Type getLightType() const override { return Light::Type::DIRECTIONAL; }

};

class SpotLight: public PointLight
{
public:
	vec3f direction;   //< spot/directional lights only

	float innerAngle;  //< the angle of spot light's inner cone.
	float outerAngle;  //< the angle of spot light's outer cone.
//	vec2f cutoff;     //< the light strength's decrease between inner and outer cone.
	
public:
	static const char* SOURCE;

	SpotLight(const vec3f& position, const vec3f& direction);
	
	
//	Light::Type getLightType() const override { return Light::Type::SPOT; }
};

}  // namespace pea
#endif  // PEA_SCENE_LIGHT_H_
