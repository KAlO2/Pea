#include "geometry/BezierPatch.h"

using namespace pea;


BezierPatch::BezierPatch(int32_t subdivision):
		curve(subdivision)
{
}

vec3f BezierPatch::bezier(const vec3f points[4], int32_t n) const
{
	assert(0 <= n && n <= getSubdivision());
	const vec4f& weight = curve.bernstein0(n);
	return sum(points, weight);
}

vec3f BezierPatch::bezier(const vec3f& point0, const vec3f& point1, const vec3f& point2, const vec3f& point3, int32_t n) const
{
	assert(0 <= n && n <= getSubdivision());
	const vec4f& weight = curve.bernstein0(n);
	return sum(point0, point1, point2, point3, weight);
}

vec3f BezierPatch::getPosition(const vec3f points[16], int32_t u, int32_t v) const
{
	vec3f column[4];
	for(uint8_t i = 0; i < 4; ++i)
		column[i] = bezier(points + (i << 2), u);
	return bezier(column, v);
}
/*
	  A  bitangent
	  |
	  |  12--13--14--15
	  |   |   |   |   |
	  |   8---9--10--11
	V |   |   |   |   |
	  |   4---5---6---7
	  |   |   |   |   |
	  |   0---1---2---3
	  |
	  +----------------->  tangent
	           U
*/
vec3f BezierPatch::getNormal(const vec3f points[16], int32_t u, int32_t v) const
{
	assert(0 <= u && u <= getSubdivision());
	assert(0 <= v && v <= getSubdivision());
	
	vec3f column[4], row[4];
	for(int8_t i = 0; i < 4; ++i)
	{
		row[i] = sum(points[i], points[i + 4], points[i + 8], points[i + 12], curve.bernstein0(v));
		column[i] = sum(points + (i << 2), curve.bernstein0(u));
	}
	
	vec3f tangent = sum(row, curve.bernstein1(u));
	vec3f bitangent = sum(column, curve.bernstein1(v));
	return cross(tangent, bitangent).normalize();
}
