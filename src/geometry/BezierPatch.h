#ifndef PEA_GEOMETRY_BEZIER_PATCH_H_
#define PEA_GEOMETRY_BEZIER_PATCH_H_

#include "math/vec3.h"
#include "geometry/BezierCurve.h"

namespace pea {

/**
 * Bezier curves are parametric curves defined by a set of n+1 control points (P0 - Pn),
 * C(t) = \sum_{i = 0}^n P_i * B_{i, n}(t);  where t b is in the range [0, 1].
 * B_{i, n}(t) = C(n, i) * t^i * (1 - t)^{n - i}
 * Here, we deal with 4x4 Bezier curves, which are bi-cubic polynomials.
 */
class BezierPatch
{
private:
	BezierCurve curve;
private:
	static vec3f sum(const vec3f points[4], const vec4f& weight);
	static vec3f sum(const vec3f& point0, const vec3f& point1, const vec3f& point2, const vec3f& point3, const vec4f& weight);

public:
	/**
	 * @param[in] subdivision Value 1 doesn't subdivide, value n tears a big patch into n^2 small 
	 *                        patches.
	 */
	explicit BezierPatch(int32_t subdivision);
	
	/**
	 * @return subdivision value, at least 1.
	 */
	int32_t getSubdivision() const;
	
	vec3f bezier(const vec3f points[4], int32_t n) const;
	vec3f bezier(const vec3f& point0, const vec3f& point1, const vec3f& point2, const vec3f& point3, int32_t n) const;
	
	/**
	 * get vertex position on a patch.
	 * @param[in] points vertex positions of a patch.
	 * @param[in] u in range [0, subdivision].
	 * @param[in] v in range [0, subdivision].
	 * @return vertex position.
	 */
	vec3f getPosition(const vec3f points[16], int32_t u, int32_t v) const;
	
	/**
	 * get vertex normal on a patch.
	 * @param[in] points vertex positions of a patch.
	 * @param[in] u in range [0, subdivision].
	 * @param[in] v in range [0, subdivision].
	 * @return vertex normal.
	 */
	vec3f getNormal(const vec3f points[16], int32_t u, int32_t v) const;
	
	/**
	 * index version of #getPosition(const vec3f[], int32_t, int32_t)
	 */
	template <typename T>
	vec3f getPosition(const vec3f* points, const T index[16], int32_t u, int32_t v) const;
	
	/**
	 * index version of #getNormal(const vec3f[], int32_t, int32_t)
	 */
	template <typename T>
	vec3f getNormal(const vec3f* points, const T index[16], int32_t u, int32_t v) const;
};

inline vec3f BezierPatch::sum(const vec3f points[4], const vec4f& weight)
{
	return weight[0] * points[0] + weight[1] * points[1] + weight[2] * points[2] + weight[3] * points[3];
}

inline vec3f BezierPatch::sum(const vec3f& point0, const vec3f& point1, const vec3f& point2, const vec3f& point3, const vec4f& weight)
{
	return weight[0] * point0 + weight[1] * point1 + weight[2] * point2 + weight[3] * point3;
}

inline int32_t BezierPatch::getSubdivision() const { return curve.getSubdivision(); }

template <typename T>
vec3f BezierPatch::getPosition(const vec3f* points, const T index[16], int32_t u, int32_t v) const
{
	vec3f column[4];
	for(int8_t i = 0; i < 16; i += 4)
		column[i >> 2] = bezier(points[index[i]], points[index[i + 1]], points[index[i + 2]], points[index[i + 3]], u);
	return bezier(column, v);
}

template <typename T>
vec3f BezierPatch::getNormal(const vec3f* points, const T index[16], int32_t u, int32_t v) const
{
	assert(0 <= u && u <= getSubdivision());
	assert(0 <= v && v <= getSubdivision());
	
	vec3f column[4], row[4];
	for(uint8_t i = 0, j = 0; i < 4; ++i, j += 4)
	{
		row[i] = sum(points[index[i]], points[index[i + 4]], points[index[i + 8]], points[index[i + 12]], curve.bernstein0(v));
		column[i] = sum(points[index[j]], points[index[j + 1]], points[index[j + 2]], points[index[j + 3]], curve.bernstein0(u));
	}
	vec3f tangent = sum(row, curve.bernstein1(u));
	vec3f bitangent = sum(column, curve.bernstein1(v));
	return cross(tangent, bitangent).normalize();
}

}  // namespace pea
#endif  // PEA_GEOMETRY_BEZIER_PATCH_H_
