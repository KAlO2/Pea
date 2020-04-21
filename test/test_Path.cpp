#include "test/catch.hpp"

#include "graphics/Path.h"
#include "util/Log.h"


using namespace pea;

static const char* tag = "[Path]";
static const char* TAG = "Path";

TEST_CASE("horizontal line", tag)
{
	constexpr int32_t N = 4;
	vec2f point0(0, 0), point1(static_cast<float>(N), 0);
	Path path;
	path.moveTo(point0).lineTo(point1);
	
	float interval[N] = {1, 1, 1, 1};
	vec4f vertex[N];
	float offset = 0;
	vec2f tangent = (point1 - point0).normalize();
	path.lineSpace(interval, vertex, N, offset);
	for(int32_t i = 0; i < N; ++i)
		REQUIRE(vertex[i] == vec4f(i, 0, tangent.x, tangent.y));
}

TEST_CASE("vertical line", tag)
{
	constexpr int32_t N = 4;
	vec2f point0(0, 0), point1(0, static_cast<float>(N));
	Path path;
	path.moveTo(point0).lineTo(point1);
	
	float interval[N] = {1, 1, 1, 1};
	vec4f vertex[N];
	float offset = 0;
	vec2f tangent = (point1 - point0).normalize();
	path.lineSpace(interval, vertex, N, offset);
	for(int32_t i = 0; i < N; ++i)
		REQUIRE(vertex[i] == vec4f(0, i, tangent.x, tangent.y));
}

TEST_CASE("line with 45 degrees", tag)
{
	constexpr int32_t N = 4;
	vec2f point0(0, 0), point1(static_cast<float>(N), static_cast<float>(N));
	Path path;
	path.moveTo(point0).lineTo(point1);
//	slog.d(TAG, "two points (%f, %f) (%f, %f) form a line\n%s", 
//			point0.x, point0.y, point1.x, point1.y, path.toString().c_str());
	
	float interval[N];
	vec4f vertex[N];
	const float step = distance(point0, point1) / N;
	for(size_t i = 0; i < N; ++i)
		interval[i] = step;
	float offset = 0;
	vec2f tangent = (point1 - point0).normalize();
	path.lineSpace(interval, vertex, N, offset);
	for(int32_t i = 0; i < N; ++i)
		REQUIRE(vertex[i] == vec4f(i, i, tangent.x, tangent.y));
}

static void testCirclePath(const vec2f center, float radius, float angle, int32_t n)
{
	assert(radius > 0 && n > 0);
	assert(std::abs(angle) > std::numeric_limits<float>::epsilon());  // angle cannot be zero.
	 
	Path path;
	path.moveTo(vec2f(center.x + radius, center.y)).arcTo(center, angle);
	
	const float step = radius * std::abs(angle) / n;
	std::vector<float> intervals(n);
	for(int32_t i = 0; i < n; ++i)
		intervals[i] = step;
	
	std::vector<vec4f> transforms(n);
	float offset = 0;
	vec4f next = path.lineSpace(intervals.data(), transforms.data(), n, offset);
	
	for(int32_t i = 0; i < n; ++i)
	{
		double theta = angle * i / n;
		float cos_a = static_cast<float>(std::cos(theta));
		float sin_a = static_cast<float>(std::sin(theta));
		
		vec2f position = center + radius * vec2f(cos_a, sin_a);
		// if angle > 0, theta + M_PI / 2;  (cos_a, sin_a) * (0, 1) = (-sin_a, cos_a);
		// if angle < 0, theat - M_PI / 2;  (cos_a, sin_a) * (0,-1) = ( sin_a,-cos_a);
		vec2f rotation = angle > 0? vec2f(-sin_a, cos_a): vec2f(sin_a, -cos_a);  
		
		const vec4f& transform = transforms[i];
		const float tolerance = 1E-6;  // std::numeric_limits<float>::epsilon();
		REQUIRE(Approx(transform.x).margin(tolerance) == position.x);
		REQUIRE(Approx(transform.y).margin(tolerance) == position.y);
		REQUIRE(Approx(transform.z).margin(tolerance) == rotation.x);
		REQUIRE(Approx(transform.w).margin(tolerance) == rotation.y);
	}
}

TEST_CASE("circle", tag)
{
	testCirclePath(vec2f(0, 0), 1, 2 * M_PI, 4);
	
	// control variate method
	testCirclePath(vec2f(0, 0), 2, 2 * M_PI, 4);
	testCirclePath(vec2f(0, 0), 1, 2 * M_PI, 10);
	testCirclePath(vec2f(1, 1), 1, 2 * M_PI, 4);
	testCirclePath(vec2f(0, 0), 1,     M_PI, 4);
	testCirclePath(vec2f(0, 0), 1,    -M_PI, 4);
	
	testCirclePath(vec2f(2, 2), 2, 4 * M_PI, 16);
}


