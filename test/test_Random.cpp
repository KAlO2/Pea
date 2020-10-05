#include "test/catch.hpp"

#include "math/Random.h"
#include "util/Log.h"

using namespace pea;

static const char* TAG = "Random";
/*
	https://www.zhihu.com/question/402778416/answer/1301785031
	Given two points on a sphere, what's the expectation of spherical distance?
*/
void verifySphericalDistance()
{
	constexpr uint32_t N = 1'000'000;
	double angleSum = 0;
	for(uint32_t i = 0; i < N; ++i)
	{
		vec3f point0 = Random::sphereEmit(1.0f);
		vec3f point1 = Random::sphereEmit(1.0f);
		double product = dot(point0, point1);
		angleSum += std::acos(product);
	}
	double angle = angleSum / N;
	
	constexpr double angleExpect = M_PI / 2;
	double error = /*std::abs*/(angle - angleExpect) / angleExpect;
	slog.i(TAG, "spherical distance expect %.6f, get %.6f, error=%.2f%%", angleExpect, angle, error * 100);
}

int main()
{
	verifySphericalDistance();
	
	return 0;
}
