#ifndef PEA_GRAPHICS_PATH_H_
#define PEA_GRAPHICS_PATH_H_

#include <cstdint>
#include <vector>
#include <string>

#include "math/vec2.h"
#include "math/vec4.h"
#include "graphics/Rect.h"

namespace pea {

/**
 * https://www.w3.org/TR/SVG/paths.html
 */
class Path
{
public:
	enum class Verb: std::uint8_t
	{
		MOVE,
		LINE,
		QUAD,
		ARC,
		CUBIC,
		CLOSE,
		DONE,
	};
	
	enum class Direction: std::uint8_t
	{
		UNKNOWN,
		CCW,
		CW,
	};

private:
	std::vector<Verb> verbs;
	std::vector<vec2f> points;
//	vec2f startPoint;

public:
	Path();
	
	/**
	 * @param[in] point0 contour start point
	 */
	Path& moveTo(const vec2f& point0);
	
	/**
	 * Add a line from the last point to the specified point.
	 * If no moveTo() call has been made for this contour, the first point is automatically set to (0,0).
	 */
	Path& lineTo(const vec2f& point1);
	
	/**
	 * Add a quadratic bezier from the last point, approaching control point point1, and ending at point2.
	 * If no moveTo() call has been made for this contour, the first point is automatically set to (0,0).
	 */
	Path& quadTo(const vec2f& point1, const vec2f& point2);
	
	/**
	 * Add a cubic bezier from the last point, approaching control points point1, point2, and ending at point3.
	 * If no moveTo() call has been made for this contour, the first point is automatically set to (0,0).
	 */
	Path& cubicTo(const vec2f& point1, const vec2f& point2, const vec2f& point3);
	
	/**
	 * @param[in] center circle center point.
	 * @param[in] sweepAngle rotate counter clockwise for positive value.
	 */
	Path& arcTo(const vec2f& center, float sweepAngle);
	
	Path& arcTo(const Rect<float>& rect, float startAngle, float sweepAngle);
	
	Path& addRect(const Rect<float>& rect, Direction direction, int32_t startIndex = 0);
	
	Path& addOval(const Rect<float>& oval, Direction direction, int32_t startIndex = 0);
	
	Path& close();
	
	std::string toString() const;
	
	/**
	 * @param[in] internal
	 * @param[out] transforms vertex's position and rotation, vec4(x, y, cos_a, sin_a),  where (x, y) is
	 *             position and (cos_a, sin_a) is tangent vector.
	 * @param[in] length length of the interval.
	 * @param[in] offset offset to the first point.
	 * @return last vertex's position and rotation
	 */
	vec4f lineSpace(const float* interval, vec4f* transforms, size_t length, float& offset) const;
};

}  // namespace pea
#endif  // PEA_GRAPHICS_PATH_H_
