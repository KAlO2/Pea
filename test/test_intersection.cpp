#include "test/catch.hpp"

#include "geometry/RayCast.h"

using namespace pea;

static const char* tag = "[collision]";  // used by Catch2

TEST_CASE("sphere", tag)
{
	Sphere sphere(vec3f(0, 0, 0), 1);
	HitInfo hitInfo;
	
	// go through center
	Ray ray(vec3f(0, -2, 0), vec3f(0, 1, 0));
	bool hit = castRay(ray, sphere, hitInfo);
	REQUIRE(hit);
	REQUIRE(hitInfo.coordinate == 1);
	REQUIRE(ray.at(hitInfo.coordinate) == vec3f(0, -1, 0));
	REQUIRE(hitInfo.normal == vec3f(0, -1, 0));
	REQUIRE(!hitInfo.inside);
	
	// opposite direction
	ray = Ray(vec3f(0, 2, 0), vec3f(0, 1, 0));
	hit = castRay(ray, sphere, hitInfo);
	REQUIRE(!hit);
	
	// ray is inside sphere
	ray = Ray(vec3f(0, 0, 0), vec3f(0, 1, 0));
	hit = castRay(ray, sphere, hitInfo);
	REQUIRE(hit);
	REQUIRE(hitInfo.coordinate == 1);
	REQUIRE(ray.at(hitInfo.coordinate) == vec3f(0, 1, 0));
	REQUIRE(hitInfo.normal == vec3f(0, -1, 0));
	REQUIRE(hitInfo.inside);
}

TEST_CASE("plane z - 1 = 0", tag)
{
	Plane plane(vec3f(0, 0, 1), -1);  // z - 1 = 0
	HitInfo hitInfo;
	
	Ray ray(vec3f(0, 0, 0), vec3f(0, 1, 0));  // parallel
	bool hit = castRay(ray, plane, hitInfo);
	REQUIRE(hit == false);
	
	ray = Ray(vec3f(0, 0, 0), vec3f(0, 0, -1));  // reverse direction
	hit = castRay(ray, plane, hitInfo);
	REQUIRE(hit == false);
	
	ray = Ray(vec3f(0, 0, 0), vec3f(0, 0, 1));  // perpendicular
	hit = castRay(ray, plane, hitInfo);
	REQUIRE(hit == true);
	REQUIRE(hitInfo.coordinate == 1);
	REQUIRE(ray.at(hitInfo.coordinate) == vec3f(0, 0, 1));
	REQUIRE(hitInfo.normal == vec3f(0, 0, -1));
	REQUIRE(!hitInfo.inside);
	
	vec3f v(1, 1, 1);
	float length = std::sqrt(3.0F);
	ray = Ray(vec3f(0, 0, 0), v / length);  // perpendicular
	hit = castRay(ray, plane, hitInfo);
	REQUIRE(hit == true);
	REQUIRE(hitInfo.coordinate == Approx(length));
	REQUIRE(ray.at(hitInfo.coordinate) == v);
	REQUIRE(hitInfo.normal == vec3f(0, 0, -1));
	REQUIRE(!hitInfo.inside);
	
	ray = Ray(vec3f(0, 0, 0), vec3f(0, 0, 1));
	hit = castRay(ray, plane, hitInfo);
	REQUIRE(hit == true);
	REQUIRE(hitInfo.coordinate == 1);
	REQUIRE(ray.at(hitInfo.coordinate) == vec3f(0, 0, 1));
	REQUIRE(hitInfo.normal == vec3f(0, 0, -1));
}

TEST_CASE("plane x + y + z - 1 = 0", tag)
{
	Plane plane(vec3f(1, 1, 1), -1);  // z - 1 = 0
	HitInfo hitInfo;
	
	Ray ray(vec3f(0, 0, 0), vec3f(-1, -1, -1).normalize());  // opposite direction
	bool hit = castRay(ray, plane, hitInfo);
	REQUIRE(hit == false);
	
	ray = Ray(vec3f(0, 0, 0), vec3f(1, 0, 0));
	hit = castRay(ray, plane, hitInfo);
	REQUIRE(hit);
	REQUIRE(hitInfo.coordinate == 1.0F);
	REQUIRE(ray.at(hitInfo.coordinate) == vec3f(1, 0, 0));
	REQUIRE(hitInfo.normal == vec3f(-1, -1, -1));
	
	ray = Ray(vec3f(1, 1, 1), vec3f(1, 0, 0));
	hit = castRay(ray, plane, hitInfo);
	REQUIRE(!hit);
	
	ray = Ray(vec3f(1, 1, 1), vec3f(-1, 0, 0));
	hit = castRay(ray, plane, hitInfo);
	REQUIRE(hit);
	REQUIRE(hitInfo.coordinate == 2.0F);
	REQUIRE(ray.at(hitInfo.coordinate) == vec3f(-1, 1, 1));
	REQUIRE(hitInfo.normal == vec3f(1, 1, 1));
}

TEST_CASE("cylinder ray along Z", tag)
{
	float radius = 1.0, height = 2.0;
	const vec3f position(0, 0, height / 2);  // cylinder on the floor
	Cylinder cylinder(radius, height);
	
	bool hit;
	HitInfo hitInfo;
	
	Ray ray(vec3f(0, -2, 0), vec3f(0, 0, 1));
	hit = castRay(ray, cylinder, position, hitInfo);
	REQUIRE(hit == false);
	
	ray.setOrigin(vec3f(0, 0.5, 0.5));
	hit = castRay(ray, cylinder, position, hitInfo);
	REQUIRE(hit == true);
	REQUIRE(hitInfo.coordinate == 1.5);
	REQUIRE(hitInfo.normal == vec3f(0, 0, -1));
	REQUIRE(hitInfo.inside);
	
	ray.setOrigin(vec3f(0, 0.5, -0.5));
	hit = castRay(ray, cylinder, position, hitInfo);
	REQUIRE(hit == true);
	REQUIRE(hitInfo.coordinate == 0.5);
	REQUIRE(hitInfo.normal == vec3f(0, 0, -1));
	REQUIRE(!hitInfo.inside);
}

TEST_CASE("cylinder ray along Y", tag)
{
	float radius = 1.0, height = 2.0;
	const vec3f position(0, 0, height / 2);  // cylinder on the floor
	Cylinder cylinder(radius, height);
	
	bool hit;
	HitInfo hitInfo;
	
	Ray ray(vec3f(0, 2, 0), vec3f(0, 1, 0));
	hit = castRay(ray, cylinder, position, hitInfo);
	REQUIRE(hit == false);
	
	ray.setOrigin(vec3f(-2, -2, 0.5));
	hit = castRay(ray, cylinder, position, hitInfo);
	REQUIRE(hit == false);
	
	ray.setOrigin(vec3f(0, 0.5, 0.5));
	hit = castRay(ray, cylinder, position, hitInfo);
	REQUIRE(hit == true);
	REQUIRE(hitInfo.coordinate == 0.5);
	REQUIRE(hitInfo.normal == vec3f(0, -1, 0));
	REQUIRE(hitInfo.inside);
}

TEST_CASE("BoundingBox", tag)
{
	BoundingBox box(vec3f(0, 0, 0), vec3f(1, 1, 1));
	bool hit;
	HitInfo hitInfo;
	
	Ray ray(vec3f(0, 2, 0), vec3f(0, 1, 0));
	hit = castRay(ray, box, hitInfo);
	REQUIRE(hit == false);
	
	ray.setOrigin(vec3f(0.5, 3, 0.5));
	hit = castRay(ray, box, hitInfo);
	REQUIRE(hit == false);
	
	ray.setOrigin(vec3f(0.5, 0.5, 0.5));
	hit = castRay(ray, box, hitInfo);
	REQUIRE(hit == true);
	REQUIRE(hitInfo.coordinate == 0.5);
	REQUIRE(hitInfo.normal == vec3f(0, -1, 0));
	REQUIRE(hitInfo.inside);
	
	ray.setDirection(vec3f(0, 0, 1));
	hit = castRay(ray, box, hitInfo);
	REQUIRE(hit == true);
	REQUIRE(hitInfo.coordinate == 0.5);
	REQUIRE(hitInfo.normal == vec3f(0, 0, -1));
	REQUIRE(hitInfo.inside);
	
	ray.setOrigin(vec3f(0.5, 0.5, -0.5));
	hit = castRay(ray, box, hitInfo);
	REQUIRE(hit == true);
	REQUIRE(hitInfo.coordinate == 0.5);
	REQUIRE(hitInfo.normal == vec3f(0, 0, -1));
	REQUIRE(!hitInfo.inside);
}
