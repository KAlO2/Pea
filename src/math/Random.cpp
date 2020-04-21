#include "math/Random.h"

#include <random>

using namespace pea;

// https://stackoverflow.com/questions/21237905/how-do-i-generate-thread-safe-uniform-random-numbers
// TODO: use template std::mt19937_64 for 64 bits?
static thread_local std::mt19937 generator(std::random_device{}());


void Random::setSeed(uint32_t seed)
{
	generator.seed(seed);
}

float Random::emit()
{
	std::uniform_real_distribution<float> distribution(0.0, 1.0);
	return distribution(generator);
}

float Random::emit(float min, float max)
{
	std::uniform_real_distribution<float> distribution(min, max);
	return distribution(generator);
}

vec3f Random::sphereEmit(float radius)
{
/*
	http://mathworld.wolfram.com/SpherePointPicking.html
	To pick a random point on the surface of a unit sphere,
	choose u and v to be random variates on [0, 1)
	theta = 2 * pi * u;     // [0, 2*pi)
	phi = acos(2 * v - 1);  // [0, pi]
*/
	using real = double;  // float
	std::uniform_real_distribution<real> distribution(0.0, 1.0);
	auto random = [&distribution](){ return distribution(generator); };
	real theta = 2 * M_PI * random();
	real phi   = std::acos(2 * random() - 1.0F);
	
	real _radius = radius * std::sin(phi);
	real x = _radius * std::cos(theta);
	real y = _radius * std::sin(theta);
	real z = radius * std::cos(phi);
	return vec3f(x, y, z);
}

vec2f Random::diskEmit(float radius)
{
/*
	http://mathworld.wolfram.com/DiskPointPicking.html
*/
	using real = double;  // float
	std::uniform_real_distribution<real> distribution(0.0, 1.0);
	auto random = [&](){ return distribution(generator); };
	real theta = 2 * M_PI * random();
	real _radius = radius * std::sqrt(random());
	real x = _radius * std::cos(theta);
	real y = _radius * std::sin(theta);
	
	return vec2f(x, y);
}

vec2f Random::circleEmit(float radius)
{
	// https://mathworld.wolfram.com/CirclePointPicking.html
	using real = double;  // float
	std::uniform_real_distribution<real> distribution(0.0, 2 * M_PI);
	real theta = distribution(generator);
	real x = radius * std::cos(theta);
	real y = radius * std::sin(theta);
	
	return vec2f(x, y);
}

