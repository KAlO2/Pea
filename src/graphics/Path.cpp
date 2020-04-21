#include "graphics/Path.h"

#include <cassert>
#include <cmath>
#include <sstream>

#include "util/Log.h"

using namespace pea;

static const char* TAG = "Path";

Path::Path()
//		startPoint(0.0F, 0.0F)
{
	verbs.push_back(Verb::MOVE);
	points.push_back(vec2f(0, 0));
}

Path& Path::moveTo(const vec2f& point0)
{
	assert(!verbs.empty());
	if(verbs[verbs.size() - 1] == Verb::MOVE)  // replace last point
		points[points.size() - 1] = point0;
	else  // append new point
	{
		verbs.push_back(Verb::MOVE);
		points.push_back(point0);
	}

	return *this;
}

Path& Path::lineTo(const vec2f& point1)
{
//	assert(!verbs.empty() && verbs[verbs.size - 1] == Verb::MOVE);
	verbs.push_back(Verb::LINE);
	points.push_back(point1);
	
	return *this;
}

Path& Path::quadTo(const vec2f& point1, const vec2f& point2)
{
	verbs.push_back(Verb::QUAD);
	points.push_back(point1);
	points.push_back(point2);
	
	return *this;
}

Path& Path::cubicTo(const vec2f& point1, const vec2f& point2, const vec2f& point3)
{
	verbs.push_back(Verb::CUBIC);
	points.push_back(point1);
	points.push_back(point2);
	points.push_back(point3);
	
	return *this;
}

Path& Path::addRect(const Rect<float>& rect, Path::Direction direction, int32_t startIndex/* = 0 */)
{
	assert(!rect.isEmpty());
	assert(direction != Path::Direction::UNKNOWN);
	
	constexpr int32_t N = 4;
	vec2f points[N] =
	{
		vec2f(rect.left,  rect.bottom),
		vec2f(rect.right, rect.bottom),
		vec2f(rect.right, rect.top),
		vec2f(rect.left,  rect.top)
	};
	
	startIndex %= N;
	moveTo(points[startIndex]);
	
	const int32_t increment = direction == Path::Direction::CCW? +1: -1;
	for(int32_t i = startIndex, endIndex = startIndex + N; i < endIndex; ++i)
		lineTo(points[i % N]);
	
	close();
	return *this;
}

Path& Path::arcTo(const vec2f& center, float sweepAngle)
{
	verbs.push_back(Verb::ARC);
	points.push_back(center);
	points.push_back(vec2f(sweepAngle, 0));
	
	return *this;
}

Path& Path::addOval(const Rect<float>& oval, Direction direction, int32_t startIndex/* = 0*/)
{
	return *this;
}

Path& Path::close()
{
#if 1
	verbs.push_back(Verb::CLOSE);
#else
	verbs.push_back(Verb::LINE);
	points.push_back(points[0]);
#endif
	return *this;
}

static constexpr int32_t advance(Path::Verb verb)
{
	using Verb = Path::Verb;
	switch(verb)
	{
	case Verb::MOVE:  return 1;
	case Verb::LINE:  return 1;
	case Verb::ARC:   return 2;  // center + radius
	case Verb::QUAD:  return 2;
	case Verb::CLOSE: return 0;
	default:  assert(false);  return 0;
	}
}

std::string Path::toString() const
{
	std::ostringstream oss;
	size_t pointIndex = 0;
	constexpr char _ = ' ';
	for(const Verb& verb: verbs)
	{
		switch(verb)
		{
		case Verb::MOVE:
			oss << "moveTo" << _ << points[pointIndex] << '\n';
			break;
		case Verb::LINE:
			oss << "lineTo" << _ << points[pointIndex] << '\n';
			break;
		case Verb::ARC:
			oss << "arcTo" << _ << "center" << points[pointIndex] << ", angle=" << points[pointIndex].x << '\n';
			break;
		case Verb::QUAD:
			oss << "quadTo" << _ << points[pointIndex] << points[pointIndex + 1] << '\n';
			break;
		case Verb::CLOSE:
			oss << "close" << '\n';
			break;
		}
		
		pointIndex += advance(verb);
	}
	
	return oss.str();
}

vec4f Path::lineSpace(const float* intervals, vec4f* transforms, size_t length, float& offset) const
{
	assert(intervals != nullptr && transforms != nullptr && length > 0);
	if(verbs.empty())
		return vec4f(0, 0, 1, 0);  // at the origin along positive X axis.
	
	size_t verbIndex = 0, pointIndex = 0, index = 0;
	vec2f lastPoint, direction;
	while(index < length)
	{
		assert(intervals[index] > 0);
		const Verb& verb = verbs[verbIndex];
		switch(verb)
		{
		case Verb::MOVE:
//			slog.d(TAG, "move pointIndex=%d, size=%d", pointIndex, points.size());
			assert(pointIndex < points.size());
			lastPoint = points[pointIndex];
			++verbIndex;
			pointIndex += advance(verb);
			break;
		case Verb::LINE:
		{
			assert(pointIndex < points.size());
			vec2 vector = points[pointIndex] - lastPoint;
			float length = vector.length();
			direction = vector / length;
			if(offset <= length)
			{
				vec2 point = lastPoint + offset * direction;
				//float angle = std::atan2(vector.y, vector.x);
//				slog.i(TAG, "%d position(%f, %f), angle=%f tangent(%f, %f)", index, point.x, point.y, direction.x, direction.y);
				transforms[index] = vec4f(point.x, point.y, direction.x, direction.y);
				offset += intervals[index];
				++index;
			}
			else
			{
				offset -= length;
				lastPoint = points[pointIndex];
				++verbIndex;
				pointIndex += advance(verb);
			}

			break;
		}
		case Verb::ARC:
		{
			const vec2f& center = points[pointIndex];
			float sweepAngle = points[pointIndex + 1].x;
			
			vec2 vector = lastPoint - center;
			float radius = vector.length();
			float arcLength = radius * std::abs(sweepAngle);
//			slog.i(TAG, "offset=%f, arcLength=%f, center(%f, %f), lastPoint(%f, %f)", offset, arcLength, center.x, center.y, lastPoint.x, lastPoint.y);
			if(offset <= arcLength)
			{
				float angle = std::copysign(offset / radius, sweepAngle);
				float cos_a = std::cos(angle);
				float sin_a = std::sin(angle);
				
				// (x, y) * (cos_a, sin_a) = (x * cos_a - y * sin_a, x * sin_a + y * cos_a);
				direction = vector / radius;
				vec2f rotation(direction.x * cos_a - direction.y * sin_a, direction.x * sin_a + direction.y * cos_a);
				vec2f point = center + radius * rotation;
				
				//angle += std::atan2(vector.y, vector.x);
				// positive, ccw (x, y)( cw 90,  cw ccw
				// rotate direction(x, y) 90 degrees, that's (-y, x) for positive sweepAngle
				// (y, -x) for negative sweepAngle.
				if(sweepAngle >= 0)
					direction = vec2f(-rotation.y, rotation.x);
				else
					direction = vec2f(rotation.y, -rotation.x);
//			slog.i(TAG, "%d position(%f, %f), angle=%f, tanget=(%f, %f)", index, point.x, point.y, angle, direction.x, direction.y);
				transforms[index] = vec4f(point.x, point.y, direction.x, direction.y);
				offset += intervals[index];
				++index;
			}
			else
			{
				offset -= arcLength;
//				slog.i(TAG, "offset=%f", offset);
				float cos_a = std::cos(sweepAngle);
				float sin_a = std::sin(sweepAngle);
				vec2f rotation(vector.x * cos_a - vector.y * sin_a, vector.x * sin_a + vector.y * cos_a);
				lastPoint = center + rotation;
				
				++verbIndex;
				pointIndex += advance(verb);
			}
			break;
		}
		
		case Verb::QUAD:
			break;
		}
		
	}
	
	return vec4f(lastPoint.x, lastPoint.y, direction.x, direction.y);
}
