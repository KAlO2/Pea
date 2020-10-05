#include "test/catch.hpp"

#include "physics/Collision.h"

using namespace pea;

static const char* tag = "[collision]";  // used by Catch2

TEST_CASE("out sphere hit", tag)
{
	Sphere sphere(vec3f(0, 0, 0), 1);
	Ray ray(vec3f(0, -2, 0), vec3f(0, 1, 0));
	
	HitInfo hitInfo;
	bool hit = Collision::castRay(ray, sphere, hitInfo);
	REQUIRE(hit == true);
	REQUIRE(hitInfo.coordinate == 1);
	REQUIRE(ray.at(hitInfo.coordinate) == vec3f(0, -1, 0));
	REQUIRE(hitInfo.normal == vec3f(0, 1, 0));
	REQUIRE(!hitInfo.inside);
	
}

TEST_CASE("out sphere miss", tag)
{
	Sphere sphere(vec3f(0, 0, 0), 1);
	Ray ray(vec3f(0, 2, 0), vec3f(0, 1, 0));
	
	HitInfo hitInfo;
	bool hit = Collision::castRay(ray, sphere, hitInfo);
	REQUIRE(hit == false);
}

TEST_CASE("in sphere", tag)
{
	Sphere sphere(vec3f(0, 0, 0), 1);
	Ray ray(vec3f(0, 0, 0), vec3f(0, 1, 0));
	
	HitInfo hitInfo;
	bool hit = Collision::castRay(ray, sphere, hitInfo);
	REQUIRE(hit == true);
	REQUIRE(hitInfo.coordinate == 1);
	REQUIRE(ray.at(hitInfo.coordinate) == vec3f(0, 1, 0));
	REQUIRE(hitInfo.normal == vec3f(0, -1, 0));
	REQUIRE(hitInfo.inside);
}

TEST_CASE("plane parallel", tag)
{
	Plane plane(vec3f(0, 0, 1), -1);  // z - 1= = 0
	Ray ray(vec3f(0, 0, 0), vec3f(0, 1, 0));
	
	HitInfo hitInfo;
	bool hit = Collision::castRay(ray, plane, hitInfo);
	REQUIRE(hit == false);
}

TEST_CASE("plane perpendicular", tag)
{
	Plane plane(vec3f(0, 0, 1), -1);  // z - 1= = 0
	Ray ray(vec3f(0, 0, 0), vec3f(0, 0, 1));
	
	HitInfo hitInfo;
	bool hit = Collision::castRay(ray, plane, hitInfo);
	REQUIRE(hit == true);
	REQUIRE(hitInfo.coordinate == 1);
	REQUIRE(ray.at(hitInfo.coordinate) == vec3f(0, 0, 1));
	REQUIRE(hitInfo.normal == vec3f(0, 0, -1));
	REQUIRE(!hitInfo.inside);
}

TEST_CASE("plane perpendicular 2", tag)
{
	Plane plane(vec3f(0, 0, -1), 1);  // z - 1= = 0
	Ray ray(vec3f(0, 0, 0), vec3f(0, 0, 1));
	
	HitInfo hitInfo;
	bool hit = Collision::castRay(ray, plane, hitInfo);
	REQUIRE(hit == true);
	REQUIRE(hitInfo.coordinate == 1);
	REQUIRE(ray.at(hitInfo.coordinate) == vec3f(0, 0, 1));
	REQUIRE(hitInfo.normal == vec3f(0, 0, -1));
}

TEST_CASE("cylinder ray along Z", tag)
{
	float radius = 1.0, height = 2.0;
	const vec3f position(0, 0, height / 2);  // cylinder on the floor
	Cylinder cylinder(radius, height);
	
	bool hit;
	HitInfo hitInfo;
	
	Ray ray(vec3f(0, -2, 0), vec3f(0, 0, 1));
	hit = Collision::castRay(ray, cylinder, position, hitInfo);
	REQUIRE(hit == false);
	
	ray.setOrigin(vec3f(0, 0.5, 0.5));
	hit = Collision::castRay(ray, cylinder, position, hitInfo);
	REQUIRE(hit == true);
	REQUIRE(hitInfo.coordinate == 1.5);
	REQUIRE(hitInfo.normal == vec3f(0, 0, -1));
	REQUIRE(hitInfo.inside);
	
	ray.setOrigin(vec3f(0, 0.5, -0.5));
	hit = Collision::castRay(ray, cylinder, position, hitInfo);
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
	hit = Collision::castRay(ray, cylinder, position, hitInfo);
	REQUIRE(hit == false);
	
	ray.setOrigin(vec3f(-2, -2, 0.5));
	hit = Collision::castRay(ray, cylinder, position, hitInfo);
	REQUIRE(hit == false);
	
	ray.setOrigin(vec3f(0, 0.5, 0.5));
	hit = Collision::castRay(ray, cylinder, position, hitInfo);
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
	hit = Collision::castRay(ray, box, hitInfo);
	REQUIRE(hit == false);
	
	ray.setOrigin(vec3f(0.5, 3, 0.5));
	hit = Collision::castRay(ray, box, hitInfo);
	REQUIRE(hit == false);
	
	ray.setOrigin(vec3f(0.5, 0.5, 0.5));
	hit = Collision::castRay(ray, box, hitInfo);
	REQUIRE(hit == true);
	REQUIRE(hitInfo.coordinate == 0.5);
	REQUIRE(hitInfo.normal == vec3f(0, -1, 0));
	REQUIRE(hitInfo.inside);
	
	ray.setDirection(vec3f(0, 0, 1));
	hit = Collision::castRay(ray, box, hitInfo);
	REQUIRE(hit == true);
	REQUIRE(hitInfo.coordinate == 0.5);
	REQUIRE(hitInfo.normal == vec3f(0, 0, -1));
	REQUIRE(hitInfo.inside);
	
	ray.setOrigin(vec3f(0.5, 0.5, -0.5));
	hit = Collision::castRay(ray, box, hitInfo);
	REQUIRE(hit == true);
	REQUIRE(hitInfo.coordinate == 0.5);
	REQUIRE(hitInfo.normal == vec3f(0, 0, -1));
	REQUIRE(!hitInfo.inside);
}
