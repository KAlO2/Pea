#ifndef PEA_MATH_MAT2X3_H_
#define PEA_MATH_MAT2X3_H_

#include "math/vec2.h"
#include "math/mat2.h"

#include <iostream>

namespace pea {

/**
 * @class mat2x3 is composition of translation/rotation/scaling operations. (affine transform)
 * In column-major mode, left multiplying is used; while in row-major mode, right multiplying
 * is used. As such, a transform A followed by a transform B can be written B*A in column-
 * major mode, or A*B in row-major mode.
 */
template <typename T>
class mat2x3
{
	/* matrix
	 * column major order   row major order
	 * [_0, _2, _4]          [_0, _1, 0]
	 * [_1, _3, _5]          [_2, _3, 0]
	 * [ 0,  0,  1]          [_4, _5, 1]
	 *
	 * rotation and scaling stored in mat2
	 * x [_0, _1]
	 * y [_2, _3]
	 *
	 * translation stored in vec2
	 * position [_4, _5]
	 *
	 */
public:
	mat2<T> basis;  //< Storage for the rotation and the scale
	vec2<T> origin; //< Storage for the translation

public:
	mat2x3(const mat2<T>& M = mat2<T>(T(1)), const vec2<T>& origin = vec2<T>(T(0)))
	:basis(M)
	,origin(origin)
	{
//		assert(M.det() >= std::numeric_limits<T>::epsilon());  // "input basis is a singular matrix"
	}

	mat2x3(
		const vec2<T>& vx,
		const vec2<T>& vy,
		const vec2<T>& origin = vec2<T>(T(0)))
	:basis(vx, vy)
	,origin(origin)
	{
		assert(!isZero<T>(basis.det()));  // "three vectors bases are coplanar"
	}

	vec2<T> getTranslation() const
	{
		return origin;
	}

	vec2<T> getScaling() const
	{
		// fast return
		if(basis.isDiagonal())
			return basis.diagonal();

		// We have to do the full calculation.
		vec2<T> v0(basis[0]), v1(basis[1]);
		return vec2<T>(v0.length(), v1.length());
	}

	vec2<T> getRotation(const vec2<T>& scale) const
	{
		// scaling negatively on two axes is the equivalent of rotating 180 degrees
		vec2<T> s(scale);
		if(s.x < 0)
		{
			if(s.y < 0)
			{
				s.x = -s.x;
				s.y = -s.y;
			}
		}
		else if(s.y < 0)
		{
			s.y = -s.y;
		}

		scale(1/s.x, 1/s.y);
		return scale;
	}

//	static transform<T> multiply(const transform<T>& lhs, const transform<T>& rhs);
	friend mat2x3<T> operator *(const mat2x3<T>& lhs, const mat2x3<T>& rhs)
	{
		mat2x3<T> result(lhs);
		result *= rhs;
		return result;
	}

#ifdef ROW_COLUMN
	friend vec2<T> operator *(const vec2<T>& position, const mat2x3<T>& transform)
	{
		return position * transform.basis + transform.origin;
	}
#else
	friend vec2<T> operator *(const mat2x3<T>& transform, const vec2<T>& position)
	{
		return transform.basis * position + transform.origin;
	}
#endif

	mat2x3<T>& operator *=(const mat2x3<T>& another)
	{
#ifdef ROW_COLUMN
		origin += another.origin * basis;
#else
		origin += basis * another.origin;
#endif
		basis *= another.basis;
		return *this;
	}

	bool operator ==(const mat2x3<T>& rhs) const
	{
		return basis == rhs.basis && origin == rhs.origin;
	}

	bool operator !=(const mat2x3<T>& rhs) const
	{
		return !(*this == rhs);
	}

	friend bool fuzzyEqual(const mat2x3<T>& lhs, const mat2x3<T>& rhs) { return lhs == rhs; }

	void loadIdentity()
	{
		basis.identity();
		origin = vec2<T>(T(0));
	}

	mat2x3<T> inverse() const
	{
		mat2<T> inv = basis.transpose();
		return mat2x3<T>(inv, inv * -origin);
	}

	/**
	 * It's equivalent to left multiplying by matrix
	 * [cos(angle) -sin(angle) 0]
	 * [sin(angle)  cos(angle) 0]
	 * [    0           0      1]
	 */
	mat2x3<T>& rotate(T angle)
	{
		const T c = std::cos(angle), s = std::sin(angle);
		T* a = basis.a;

		T tmp= c*a[2] - s*a[0];
		a[0] = s*a[2] + c*a[0];
		a[2] = tmp;

		tmp  = s*a[3] + c*a[4];
		a[3] = c*a[3] - s*a[4];
		a[4] = tmp;

		tmp     = s*origin.x + c*origin.y;
		origin.x = c*origin.x - s*origin.y;
		origin.y = tmp;

		return *this;
	}

	/**
	 * It's equivalent to left multiplying matrix
	 * [1 0 x]
	 * [0 1 y]
	 * [0 0 1]
	 */
	mat2x3<T>& translate(const vec2<T>& v)
	{
		origin += v;
		return *this;
	}

	/**
	 * It's equivalent to left multiplying matrix
	 * [x 0 0]
	 * [0 y 0]
	 * [0 0 1]
	 */
	mat2x3<T>& scale(const vec2<T>& v)
	{
		T* a = basis.a;
		a[0] *= v.x; a[2] *= v.x;
		a[1] *= v.y; a[3] *= v.y;
		return *this;
	}

	template <typename charT, class traits>
	friend std::basic_ostream<charT, traits>& operator <<(
			std::basic_ostream<charT, traits>& os, const  mat2x3<T>& xform)
	{
		std::basic_ostringstream<charT, traits> stream;
		stream.flags(os.flags());
		stream.imbue(os.getloc());
		stream.precision(os.precision());
		stream << "basis:\n" << xform.basis << "origin:\n" << xform.origin;
		return os << stream.str();
	}
};

using mat2x3f = mat2x3<float> ;
using mat2x3d = mat2x3<double>;

}  // namespace pea
#endif  // PEA_MATH_MAT2X3_H_
