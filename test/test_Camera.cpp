#include "test/catch.hpp"

#include "geometry/Sphere.h"
#include "scene/Camera.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static const char* tag = "Camera";

using namespace pea;

// column major
static void print(const glm::mat4& m)
{
	constexpr int8_t ORDER = 4;
	for(int8_t i = 0; i < ORDER; ++i)
	{
		std::cout << '[' << ' ';
		for(int8_t j = 0; j < ORDER; ++j)
			std::cout << std::setw(12) << std::setfill(' ') << m[i][j] << ' ';
		std::cout << ']' << std::endl;
	}
}

static bool fuzzyEqual(const pea::vec4f& v1, const glm::vec4& v2)
{
	return fuzzyEqual(v1.x, v2.x) && fuzzyEqual(v1.y, v2.y) && 
			fuzzyEqual(v1.z, v2.z) && fuzzyEqual(v1.w, v2.w);
}

static bool fuzzyEqual(const pea::mat4f& m1, const glm::mat4& m2)
{
	for(int8_t i = 0; i < 4; ++i)
	for(int8_t j = 0; j < 4; ++j)
	{
#if COLUMN_MAJOR
		if(!fuzzyEqual(m1[i][j], m2[i][j]))
#else
		if(!fuzzyEqual(m1[j][i], m2[i][j]))
#endif
		{
			std::cout << m1 << '\n';
			print(m2);
			return false;
		}
	}

	return true;
}

pea::mat4f difference(const pea::mat4f& m1, const glm::mat4& m2)
{
	pea::mat4f result;
	for(int8_t i = 0; i < 4; ++i)
		for(int8_t j = 0; j < 4; ++j)
#if COLUMN_MAJOR
			result[i][j] = m1[i][j] - m2[i][j];
#else
			result[j][i] = m1[j][i] - m2[i][j];
#endif

	return result;
}

inline glm::vec3 glm_cast(const pea::vec3f& v) noexcept
{
	return glm::vec3(v.x, v.y, v.z);
}

inline glm::vec4 glm_cast(const pea::vec4f& v) noexcept
{
	return glm::vec4(v.x, v.y, v.z, v.w);
}

TEST_CASE("transform", tag)
{
	float angle = static_cast<float>(M_PI / 3);
	REQUIRE(angle == glm::pi<float>() / 3);
	
	mat4f transform1(1.0F);
	glm::mat4 transform2(1.0F);
	REQUIRE(fuzzyEqual(transform1, transform2));
	
	vec3f offset(1.0F, 2.0F, 3.0F);
	transform1.translate(offset);
	transform2 = glm::translate(transform2, glm_cast(offset));
	REQUIRE(fuzzyEqual(transform1, transform2));
	
	vec3f normal(1.0F, 0.3F, 0.5F);
	mat4f rotation1 = mat4f::rotate(normal.normalize(), angle);
	glm::mat4 rotation2 = glm::rotate(glm::mat4(1.0F), angle, glm_cast(normal));
	REQUIRE(fuzzyEqual(rotation1, rotation2));
	
	vec3f factor(2.0F, 3.0F, 4.0F);
	mat4f scale1(factor.x, factor.y, factor.z, 1.0F);
	glm::mat4 scale2 = glm::scale(glm::mat4(1.0F), glm_cast(factor));
	REQUIRE(fuzzyEqual(scale1, scale2));
	
	// left multiply for column major layout, right multiply for row major layout.
	// modelViewProjection = projection * view * model;  // column major
	// modelViewProjection = model * view * projection;  // row major
#if COLUMN_MAJOR
	transform1 = scale1 * rotation1 * transform1;
#else
	transform1 = transform1 * rotation1 * scale1;
#endif
	transform2 = scale2 * rotation2 * transform2;
	REQUIRE(fuzzyEqual(transform1, transform2));
	
	// glm always use column major layout.
	// gl_Position = modelViewProjection * vec4(position, 1.0)
	vec4f position(1.0F, 2.0F, 3.0F, 1.0F);
#if COLUMN_MAJOR
	vec4f position1 = transform1 * position;
#else
	vec4f position1 = position * transform1;
#endif
	glm::vec4 position2 = transform2 * glm_cast(position);
	REQUIRE(fuzzyEqual(position1, position2));
}

TEST_CASE("lookAt", tag)
{
	vec3f position(5.0f, 3.0f, 4.0f);
	vec3f forward(1.0f, 2.0f, 3.0f);
	vec3f up(0.4f, 1.0f, 0.8f);
	vec3f target = position + forward * 3.0f;
	
	mat4f transform1 = Camera::lookAt(position, target, up);
	glm::mat4 transform2 = glm::lookAt(glm_cast(position), glm_cast(target), glm_cast(up));
	REQUIRE(fuzzyEqual(transform1, transform2));
	
	position = vec3f(0.0f, 0.0f, 3.0f);
	forward = vec3f(0.0f, 0.0f, -1.0f);
	up = vec3f(0.0f, 1.0f, 0.0f);
	
	transform1 = Camera::lookAt(position, position + forward, up);
	transform2 = glm::lookAt(glm_cast(position), glm_cast(position + forward), glm_cast(up));
	REQUIRE(fuzzyEqual(transform1, transform2));

}

TEST_CASE("perspective", tag)
{
	float fov = static_cast<float>(M_PI / 3);
	float aspect = 16.0F / 9;  // width:height = 16:9
	float near = 1.0f;
	float far = 1024.0f;

	mat4f transform1 = Camera::perspective(fov, aspect, near, far);
	glm::mat4 transform2 = glm::perspective(fov, aspect, near, far);
	REQUIRE(fuzzyEqual(transform1, transform2));
}

TEST_CASE("orbit", tag)
{
	vec3f position(0, -1, 0), pivot(0, 0, 0), up(0, 0, 1);
	Camera camera(position, pivot, up);
	
	const vec3f& forward = camera.getForward();
	REQUIRE(forward == vec3f(0, 1, 0));
	
	// notice the negative sign, orbit is made relative to target, not position
	vec3f orientation = -forward;
	auto [pitch, yaw] = Sphere::decomposeOrientation(orientation);
	REQUIRE(pitch == Approx(0.0));
	REQUIRE(yaw == Approx(-M_PI / 2));
	
	camera.orbit(pivot, pitch, yaw);
	REQUIRE(camera.getForward() == forward);
	REQUIRE(camera.getPosition() == vec3f(0, -1, 0));
	
	yaw = 0;
	camera.orbit(pivot, pitch, yaw);
	REQUIRE(camera.getForward() == vec3f(-1, 0, 0));
	REQUIRE(camera.getPosition() == vec3f(1, 0, 0));
	
}
