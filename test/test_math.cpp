#include "test/catch.hpp"

#include "math/function.h"
#include "math/hash.h"
#include "math/mat2.h"
#include "math/mat2x3.h"
#include "math/mat3.h"
#include "math/mat3x4.h"
#include "math/mat4.h"
#include "math/quaternion.h"
#include "math/scalar.h"
#include "math/Transform.h"
#include "math/vec2.h"
#include "math/vec3.h"
#include "math/vec4.h"


using namespace pea;

static const char* tag = "[math]";

TEST_CASE("scalar", tag)
{
//	EXPECT_FALSE(isPowerOfTwo(-1L));  // will cause assertion failed
	REQUIRE(!isPowerOfTwo(0));
	REQUIRE(isPowerOfTwo(2));
	REQUIRE(isPowerOfTwo(0x100));
/*
	REQUIRE(nextPowerOfTwo(0) == 1);
	REQUIRE(nextPowerOfTwo(1) == 1);
	REQUIRE(nextPowerOfTwo(uint8_t(127))  == uint8_t(128));
	REQUIRE(nextPowerOfTwo(int32_t(4000)) == int32_t(4096));
*/
	REQUIRE(Approx(M_PI)  == deg2rad<double>(180.0));
	REQUIRE(Approx(180.0) == rad2deg<double>(M_PI));
#if __cplusplus >= 201103L
	REQUIRE(Approx(M_PI)  == static_cast<double>(180.0_deg));
#endif
	REQUIRE(Approx(1.0) == std::asin(std::sin(1.0)));

	double x = 180.0;
	REQUIRE(Approx(1.0) == std::cos(x) * std::cos(x) + std::sin(x) * std::sin(x));
}

TEST_CASE("vec2", tag)
{
	const double theta = M_PI / 3.0;

	vec2d v0(0.0, 0.0);
	vec2d v1(std::cos(theta), std::sin(theta));
	vec2d v2 = v0 - v1;
	vec2d v3(3.0, 3.0);

	REQUIRE(Approx(0.0) == v0.length());
	REQUIRE(Approx(1.0) == v1.length());
	REQUIRE(Approx(1.0) == v2.length());

	REQUIRE(v0 == v1 + v2);

	REQUIRE(std::isnan(angle(v0, v1)));
	REQUIRE(Approx(0.0) == angle(v1, v1));
	REQUIRE(Approx(M_PI) == angle(v1, v2));
	REQUIRE(Approx(theta) == angle(v1, vec2d(1.0, 0.0)));

	v2 = -v2;
	REQUIRE(v1 == v2);
	REQUIRE(Approx(M_PI / 12) == angle(v2, v3));  //  15.0_deg

	v3 += vec2d(0.0, 1.0);
	REQUIRE(vec2d(3.0, 4.0) == v3);
	REQUIRE(5.0 == v3.length());

	v3.rotate(M_PI / 2);
	REQUIRE(vec2d(-4.0, 3.0) == v3);

	v3.rotate(M_PI);
	REQUIRE(vec2d(4.0, -3.0) == v3);

	v3.normalize();
	REQUIRE(vec2d(0.8, -0.6) == v3);
}

TEST_CASE("vec3", tag)
{
	using Vec3 = vec3f;
	constexpr Vec3::value_type ZERO = 0;
	constexpr Vec3::value_type ONE = 1;

	Vec3 right(ONE, ZERO, ZERO);
	Vec3 forward(ZERO, ONE, ZERO);
	Vec3::value_type unit_z[] = {ZERO, ZERO, ONE};
	Vec3 up(unit_z);

	Vec3 v = right + forward + up;

	REQUIRE(ONE == v.x);
	REQUIRE(std::sqrt(3 * ONE) == v.length());
	REQUIRE(Approx(M_PI / 2) == angle(right, forward));

	v = right;
	REQUIRE(Vec3(ONE, ZERO, ZERO) == v);
	REQUIRE(forward == v.rotateZ(M_PI / 2));
	REQUIRE(up == cross(right, forward));
	REQUIRE(ZERO == dot(right, forward));

	v.normalize();
	REQUIRE(ONE == v.length());

	Vec3 vx = v.project(right);
	Vec3 vy = v.project(forward);
	Vec3 vz = v.project(up);
	REQUIRE(v == vx + vy + vz);
	
	// coordinate
	REQUIRE(polar_cast(right) == right);
	REQUIRE(cartesian_cast(right) == right);
	
	Vec3 forwardPolar(ONE, M_PI / 2, 0);
	REQUIRE(polar_cast(forward) == forwardPolar);
	REQUIRE(cartesian_cast(forwardPolar) == forward);
	
	Vec3 c(ONE, ONE, ONE), p(std::sqrt(3.0), M_PI / 4, std::atan(std::sqrt(2) / 2.0));
	REQUIRE(polar_cast(c) == p);
	REQUIRE(cartesian_cast(p) == c);
}

TEST_CASE("vec4", tag)
{
	vec4i v(1, 2, 3, 4);
	REQUIRE((1 + 4 + 9 + 16) == dot(v, v));
	REQUIRE(vec4i(2, 4, 6, 8) == v + v);
}

TEST_CASE("mat2", tag)
{
	//  [2  0]   inverse   [  1/2,    0]
	//  [3  7]  -------->  [-3/14, 2/14]
	mat2d m(2, 0, 3, 7);
	mat2d m2(0.5, 0, -3.0/14, 2.0/14); // inverse of M calculated by GNU Octave
	REQUIRE(3.0 == m[0][1]);
	REQUIRE(mat2d(1.0, 0.0, -3.0/7, 2.0/7) == m2 * 2.0);

	mat2d inv_m = m.inverse();
	REQUIRE(m2 == inv_m);
}

TEST_CASE("mat3", tag)
{
/*
	M = [8,3,4; 1,5,9; 6,7,2;];
	det(M) = -360
	inv(M) =
		0.147222  -0.061111  -0.019444
		-0.144444   0.022222   0.188889
		0.063889   0.105556  -0.102778
*/
	using real  = double;
	using vec3r = vec3<real>;
	using mat3r = mat3<real>;

	const real array[] = {8,3,4, 1,5,9, 6,7,2};  // row major
	mat3r m1(array);
	if(column_major)
		m1.transpose();  // change C's row major to OpenGL's column major.

	REQUIRE(m1[1][2] == static_cast<real>(column_major ? 7:9));
#if COLUMN_MAJOR
	REQUIRE(m1[1] == vec3r(3.0, 5.0, 7.0));
#else
	REQUIRE(m1[1] == vec3r(1.0, 5.0, 9.0));
#endif

	mat3r m2 = m1;
	m2.transpose();
	REQUIRE(m2[2][1] == static_cast<real>(column_major ? 7:9));

	const real VALUE = -360.0;
	REQUIRE(VALUE == m1.determinant());
	REQUIRE(VALUE == m2.determinant());

	mat3r m = m1 * m2;
	REQUIRE(m.determinant() == m1.determinant() * m2.determinant());

	m.identity();
	REQUIRE(static_cast<mat3r::value_type>(1) == m.determinant());

	m2 = m1;
	m1 = m1.inverse();
	REQUIRE(m == m1 * m2);
	REQUIRE(m == m2 * m1);

/*
octave:1> M = [1, 3, 6; 8, 2, 6; 4, 7, 8]
M =

   1   3   6
   8   2   6
   4   7   8

octave:2> det(M)
ans =  142
octave:3> inv(M)
ans =

  -0.183099   0.126761   0.042254
  -0.281690  -0.112676   0.295775
   0.338028   0.035211  -0.154930

octave:4> M*M*M
ans =

    745    753   1176
   1040   1040   1572
   1268   1350   2084

octave:5> v = [1, 2, 4]';
octave:6> M*v
ans =

   31
   36
   50

*/
	m = mat3r(
		1, 3, 6,
		8, 2, 6,
		4, 7, 8);
	vec3d v(1, 2, 4);

	mat3d m_inverse(
		-0.183099,   0.126761,   0.042254,
		-0.281690,  -0.112676,   0.295775,
		 0.338028,   0.035211,  -0.154930);
	mat3d mmm(
		 745,    753,   1176,
		1040,   1040,   1572,
		1268,   1350,   2084);
	vec3d mv(31.0, 36.0, 50.0);

	REQUIRE(142.0F == m.determinant());
	REQUIRE(m_inverse == m.inverse());

	REQUIRE(mmm == m * m * m);
#if COLUMN_MAJOR
	REQUIRE(mv == m * v);
#else
	REQUIRE(mv == v * m);
#endif
}

TEST_CASE("mat4", tag)
{
	const float ONE = 1;
	mat4f I(ONE);
	REQUIRE(ONE == I.determinant());

/*
octave:1> M = [1,2,3,4; 2,4,6,7; 7,7,5,2; 8,3,6,9;];
octave:2> det(M)
ans =  82
octave:3> inv(M)
ans =

  -0.25610  -0.03659   0.03659   0.13415
   2.46341  -1.21951   0.21951  -0.19512
  -3.89024   2.15854  -0.15854   0.08537
   2.00000  -1.00000   0.00000   0.00000

octave:4> M*M
ans =

    58    43    54    60
   108    83   102   111
    72    83   100   105
   128    97   126   146

octave:5> v = [1,2,4,8]';
octave:6> M*v
ans =

    49
    90
    57
   110

*/
	mat4f m(
		1, 2, 3, 4,
		2, 4, 6, 7,
		7, 7, 5, 2,
		8, 3, 6, 9);
	mat4f inv_m(
		-0.25610,  -0.03659,   0.03659,   0.13415,
		 2.46341,  -1.21951,   0.21951,  -0.19512,
		-3.89024,   2.15854,  -0.15854,   0.08537,
		 2.00000,  -1.00000,   0.00000,   0.00000);
	mat4f mm(
		 58, 43,  54,  60,
		108, 83, 102, 111,
		 72, 83, 100, 105,
		128, 97, 126, 146);
	
	const vec4f v(1, 2, 4, 8);
	vec4f mv(49.0, 90.0, 57.0, 110.0);
	
	REQUIRE(m[0][2] == (column_major?7:3));
	REQUIRE(m.determinant() == 82);
	REQUIRE(inv_m == m.inverse());
	REQUIRE(mm == m * m);
#if COLUMN_MAJOR
	REQUIRE(mv == m * v);
#else
	REQUIRE(mv == v * m);
#endif

	// association law
	REQUIRE(I == m * inv_m);
	REQUIRE(I == inv_m * m);
	REQUIRE(m * inv_m * mm == m * (inv_m * mm));
	
	float angle = 0.12345;
	const mat4f m2 = m;
	REQUIRE(mat4f::getRotationX(angle) * m2 == m.rotateX(angle));
	m = m2;
	REQUIRE(mat4f::getRotationY(angle) * m2 == m.rotateY(angle));
	m = m2;
	REQUIRE(mat4f::getRotationZ(angle) * m2 == m.rotateZ(angle));
}

TEST_CASE("translate rotate scale", tag)
{
	const float x = 2, y = 3, z = 4;
	vec3f v(x, y, z); 
	mat4f m1 = mat4f(1.0).translate(v);
	mat4f m1Expect(
		1, 0, 0, x,
		0, 1, 0, y,
		0, 0, 1, z,
		0, 0, 0, 1);
	REQUIRE(m1 == m1Expect);
	
	mat4f m2 = mat4f(1.0).scale(v);
	mat4f m2Expect(
		x, 0, 0, 0,
		0, y, 0, 0,
		0, 0, z, 0,
		0, 0, 0, 1);
	REQUIRE(m2 == m2Expect);
	
}
/*
TEST_CASE("mat2x3", tag)
{
	// affine transform

}

TEST_CASE("mat3x4", tag)
{
	mat3x4f t1, t2;

	vec3f position(1.0, 1.0, 1.0);
	t1.translate(position);
	REQUIRE(position == t1.origin);

	t1.loadIdentity();
	t2.loadIdentity();

	const float angle = M_PI / 2;
	t1 *= rotate(vec3f(1.0, 0, 0), angle);
	t2.rotateX(angle);
	REQUIRE(t1 == t2);

	t1.rotate(vec3f(0, 1.0, 0), angle);
	t2.rotateY(angle);
	REQUIRE(t1 == t2);

	t1.rotate(vec3f(0, 0, 1.0), angle);
	t2.rotateZ(angle);
	REQUIRE(t1 == t2);

	// Ry(pi)*Rx(pi) == Rz(pi), where Rx, Ry, Rz denotes rotation on X/Y/Z axis sperately.
	t1.loadIdentity();
	t2.loadIdentity();
	t1.rotateX(M_PI).rotateY(M_PI);
	t2.rotateZ(M_PI);
	REQUIRE(t1 == t2);
}
*/
TEST_CASE("quaternion", tag)
{
	vec3f gimbal(0.34f, 0.56f, 0.78f);  // pitch, roll, yaw angle
	mat4f pitch(1.0f);  pitch.rotateX(gimbal.pitch);
	mat4f roll(1.0f);   roll.rotateY(gimbal.roll);
	mat4f yaw(1.0f);    yaw.rotateZ(gimbal.yaw);
	
	mat4f result = yaw * roll * pitch;

	quaternionf q(1.0f, 2.0f, 3.0f, 4.0f);
	REQUIRE(std::sqrt(30.f) == q.length());

	vec3f v(1.0, 1.0, 1.0);
	v.normalize();
	REQUIRE(vec3f(1/sqrt(3.0)) == v);

	// http://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation#Example
	// let's construct a permutation group (xyz)->(zxy) to observe how the
	// three axes are permuted cyclically.

#if __cplusplus >= 201103L
	// Note that any space between number and string literal will cause an compiler error!
	float angle = 120.0_deg;
#else
	float angle = 120.0f / 180.0f * M_PI;
#endif
	q = quaternionf(v, angle);

	REQUIRE(quaternionf(0.5, 0.5, 0.5, 0.5) == q);
/*
	REQUIRE(Approx(0.5) == q.w);
	REQUIRE(Approx(0.5) == q.x);
	REQUIRE(Approx(0.5) == q.y);
	REQUIRE(Approx(0.5) == q.z);
*/
//	float a = 3.141'592'653'589'793'238'462'643'383'279'502'88;
//	float b = 2.718'281'828'459'045;
//	float c = 0.577'215'664'901'532'860'60;
	float a = 3.14159265358979323846264338327950288;
	float b = 2.718281828459045;
	float c = 0.57721566490153286060;
	const vec3f abc(a, b, c), cab(c, a, b), a_cb(a, -c, b);
#if COLUMN_MAJOR
	REQUIRE(cab == q.mat3_cast() * abc);
#else
	REQUIRE(cab == abc * q.mat3_cast());
#endif
	// rotate x axis pi/2 radians counter-clockwise.
	q = quaternionf(0, abc);  // a random number
	quaternionf p(vec3f(1.0F, 0, 0), static_cast<float>(M_PI/2));  // rotate X axis 90 degrees.

#if COLUMN_MAJOR
	REQUIRE(a_cb == p.mat3_cast() * abc);
#else
	REQUIRE(a_cb == abc * p.mat3_cast());
#endif
	REQUIRE(a_cb == vec3f(abc).rotateX(a/2));
//	REQUIRE(quaternionf(0, a, -c, b), );
//	std::cout << vec3f(a, -c, b) << '\n' << q.rotate(p);
}

TEST_CASE("generate sine cosine table full", tag)
{
	constexpr uint8_t N = 5;
	float cost[N] = {0}, sint[N] = {0};
	generateCosineSineTable(cost, sint, N - 1, false/* halfCircle */);
	
	float sintExpect[N] = {0, +1, 0, -1, 0};
	float costExpect[N] = {+1, 0, -1, 0, +1};
	const float tolerance = std::numeric_limits<float>::epsilon();
	for(uint8_t i = 0; i < N; ++i)
	{
		REQUIRE(sint[i] == Approx(sintExpect[i]).margin(tolerance));
		REQUIRE(cost[i] == Approx(costExpect[i]).margin(tolerance));
	}
}

TEST_CASE("generate sine cosine table half", tag)
{
	constexpr uint8_t N = 5;
	float cost[N] = {0}, sint[N] = {0};
	generateCosineSineTable(cost, sint, N - 1, true/* halfCircle */);
	
	const float a = static_cast<float>(std::sqrt(2.0) / 2);
	float sintExpect[N] = {0, +a, 1, +a, 0};
	float costExpect[N] = {+1, +a, 0, -a, -1};
	const float tolerance = std::numeric_limits<float>::epsilon();
	for(uint8_t i = 0; i < N; ++i)
	{
		REQUIRE(sint[i] == Approx(sintExpect[i]).margin(tolerance));
		REQUIRE(cost[i] == Approx(costExpect[i]).margin(tolerance));
	}
}

TEST_CASE("integer power", tag)
{
	REQUIRE(pea::pow(2, 0) == 1);
	REQUIRE(pea::pow(2, 1) == 2);
	REQUIRE(pea::pow(2, 3) == 8);
	REQUIRE(pea::pow<uint32_t>(2u, 3u) == 8u);
}

