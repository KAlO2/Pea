#ifndef PEA_GEOMETRY_RAY_CAST_H_
#define PEA_GEOMETRY_RAY_CAST_H_

#include "geometry/BoundingBox.h"
#include "geometry/Cylinder.h"
#include "geometry/Plane.h"
#include "geometry/Ray.h"
#include "geometry/Sphere.h"
#include "math/mat4.h"

namespace pea {

/**
 * 1. vec3 normal; float t; bool inside;
 *    normal is with respect to ray position (inside or outside geometry).
 * 2. vec3 normal, float t1, t2;
 *    normal is always point outwards.
 */
class HitInfo
{
public:
	float coordinate;  // vec3f hitPoint = ray.at(t);
	vec3f normal;  // surface normal, with respect to the ray
	bool inside;  // ray is inside geometry or not
};

/**
 * For the incident vector and surface orientation, returns the reflection direction.
 * @param[in] incident If incident is normalized, reflection direction will be normalized.
 * @param[in] normal   Normal must already be normalized in order to achieve the desired result.
 */
vec3f reflect(const vec3f& incident, const vec3f& normal);

/**
 * For the incident vector and surface normal, and the ratio of indices of refraction eta, 
 * return the refraction vector.
 * @param[in] incident Incident must already be normalized.
 * @param[in] normal   Normal must already be normalized.
 * @param[in] eta      Refractive index gives the speed of light within a medium compared to the
 *                     speed of light within a vacuum. vacuum = 1.0, air = 1.000277, water = 4/3
 *                     ice = 1.31, glass = 1.5, diamond = 2.417
 *
 * @see https://docs.blender.org/manual/es/2.79/render/blender_render/materials/properties/transparency.html#ior-values-for-common-materials
 */
vec3f refract(const vec3f& incident, const vec3f& normal, const float& eta);

float fresnelReflectance(const vec3f& I, const vec3f& N, float eta);

float schlickReflectance(const vec3f& I, const vec3f& N, float eta);

/**
 * Get the closest point on this ray to a point
 * @param[in] ray
 * @param[in] point The point to test.
 * @return The nearest point which is in the ray.
 */
vec3f getClosestPoint(const Ray& ray, const vec3f& point);

/**
 * Test if ray intersects sphere s
 */
bool castRay(const Ray& ray, const Sphere& sphere, HitInfo& hitInfo);

bool castRay(const Ray& ray, const Plane& plane, HitInfo& hitInfo);

/**
 * @param[in] position cylinder's position
 */
bool castRay(const Ray& ray, const Cylinder& cylinder, const vec3f& position, HitInfo& hitInfo);

/**
 * Test whether this ray intersects with the given AABB(Axis Aligned Bounding Box)
 * @param box The AABB to be tested
 * @param t if intersects and t is not null, calculate the distance along the ray
 *        at which it intersects.
 * @return true indicates an intersection occurs, otherwise false.
 */
bool castRay(const Ray& ray, const BoundingBox& box, HitInfo& hitInfo);

bool castRay(const Ray& ray, const vec3f vertices[3], HitInfo& hitInfo);

/**
 * @param[in] quadric 
 *
 * f(x, y, z) = ax^2 + 2bxy + 2cxz + 2dx + ey^2 + 2fyz + 2gy + hz^2 + 2iz +j = 0
 *           [a b c d] [x]
 * [x y z 1] [b e f g] [y] = 0
 *           [c f h i] [z]
 *           [d g i j] [1]
 */
bool castRay(const Ray& ray, const mat4f& quadric, HitInfo& hitInfo);

}  // namespace pea
#endif  // PEA_GEOMETRY_RAY_CAST_H_
