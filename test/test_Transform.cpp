#include "test/catch.hpp"

#include "math/Transform.h"


using namespace pea;
static const char* tag = "[math]";

TEST_CASE("Transform::Tanslate", tag)
{
	Transform transform;
	transform.translation = vec3f(-2, -4, -3);
	
	mat4f matrixExpect(
			1, 0, 0, -2,
			0, 1, 0, -4,
			0, 0, 1, -3,
			0, 0, 0, +1);
	
	REQUIRE(matrixExpect == transform.getTransform());
}

TEST_CASE("Transform::rotate", tag)
{
	const float t = M_PI / 3;
	const float cos_t = std::cos(t);
	const float sin_t = std::sin(t);
	
	Transform transform;
	
	transform.rotation = vec3f(t, 0, 0);
	mat4f matrix0Expect(
			1, 0, 0, 0,
			0, cos_t, -sin_t, 0,
			0, sin_t, +cos_t, 0,
			0, 0, 0, 1);
	REQUIRE(matrix0Expect == transform.getTransform());
	
	transform.rotation = vec3f(0, t, 0);
	mat4f matrix1Expect(
			cos_t, 0, sin_t, 0,
			0, 1, 0, 0,
			-sin_t, 0, cos_t, 0,
			0, 0, 0, 1);
	REQUIRE(matrix1Expect == transform.getTransform());
	
	transform.rotation = vec3f(0, 0, t);
	mat4f matrix2Expect(
			cos_t, -sin_t, 0, 0,
			sin_t, +cos_t, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
	REQUIRE(matrix2Expect == transform.getTransform());
}

TEST_CASE("Transform::scale", tag)
{
	Transform transform;
	transform.scaling = vec3f(2, 3, 4);
	
	mat4f matrixExpect(
			2, 0, 0, 0,
			0, 3, 0, 0,
			0, 0, 4, 0,
			0, 0, 0, 1);
	
	REQUIRE(matrixExpect == transform.getTransform());
}

TEST_CASE("Transform::TRS", tag)
{
	Transform transform;
	transform.translation = vec3f(1, 2, 3);
	transform.rotation = vec3f(M_PI * 2 + M_PI / 2, 0, 0);
	transform.scaling = vec3f(2, 3, 4);
/*
	[1,  ,  , 1]   [1,  ,  ,  ]   [2,  ,  ,  ]   
	[ , 1,  , 2] * [ ,  ,-1, 1] * [ , 3,  ,  ] = 
	[ ,  , 1, 3]   [ , 1,  , 1]   [ ,  , 4,  ]   
	[ ,  ,  , 1]   [ ,  , 0, 1]   [ ,  ,  , 1]   
*/
	mat4f matrixExpect(
			2, 0, 0, 1,
			0, 0,-4, 2,
			0, 3, 0, 3,
			0, 0, 0, 1);
	
	REQUIRE(matrixExpect == transform.getTransform());
}

TEST_CASE("Transform::inverse", tag)
{
	Transform transform;
	transform.translation = vec3f(1, 2, 3);
	transform.rotation = vec3f(M_PI / 6, 0, M_PI/ 2);
	transform.scaling = vec3f(0.5, 0.4, 0.2);
	
	mat4f m1 = transform.getTransform();
	mat4f m2 = transform.getInverseTransform();
	REQUIRE(mat4f(1.0F) == m1 * m2);
	REQUIRE(mat4f(1.0F) == m2 * m1);
}

