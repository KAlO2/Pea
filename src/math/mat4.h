#ifndef PEA_MATH_MAT4_H_
#define PEA_MATH_MAT4_H_

#include <iostream>

#include "pea/config.h"
#include "math/mat3.h"
#include "math/vec4.h"


namespace pea {

template <typename T>
class alignas(4 * sizeof(T)) mat4
{
public:
	union
	{
		vec4<T> v[4];
		T a[16];
	};

	typedef T value_type;
	static constexpr int8_t ORDER = 4;
//	static constexpr mat4<T> ONE = mat4<T>(T(1));
public:
	mat4(): mat4(static_cast<T>(1)) { }
	~mat4() = default;
	mat4(const mat4<T>& M) = default;  // { memcpy(a, M.a, sizeof(a)); }
	mat4<T>& operator =(const mat4<T>& rhs) = default;

	explicit mat4(const T* array)
	{
//		memcpy(a, array, sizeof(a));
		for(int8_t i = 0; i < (ORDER * ORDER); ++i)
			a[i] = array[i];
	}

	explicit mat4(T diagonal)
	{
		for(int8_t i = 1; i < 15; ++i)
			a[i] = static_cast<T>(0);
		a[15] = a[10] = a[5] = a[0] = diagonal;
	}

	explicit mat4(T d0, T d1, T d2, T d3)
	{
		for(int8_t i = 1; i < 15; ++i)
			a[i] = static_cast<T>(0);
		a[0] = d0;
		a[5] = d1;
		a[10]= d2;
		a[15]= d3;
	}
	
	explicit mat4(const vec4<T>& v0, const vec4<T>& v1, const vec4<T>& v2, const vec4<T>& v3)
	{
		v[0] = v0;
		v[1] = v1;
		v[2] = v2;
		v[3] = v3;
	}
	
	mat4(T _00, T _01, T _02, T _03,
	     T _10, T _11, T _12, T _13,
	     T _20, T _21, T _22, T _23,
	     T _30, T _31, T _32, T _33)
	{
#if COLUMN_MAJOR  // if constexpr(column_major) needs indentation
		a[0] = _00; a[4] = _01; a[8]  = _02; a[12] = _03;
		a[1] = _10; a[5] = _11; a[9]  = _12; a[13] = _13;
		a[2] = _20; a[6] = _21; a[10] = _22; a[14] = _23;
		a[3] = _30; a[7] = _31; a[11] = _32; a[15] = _33;
#else
		a[0]  = _00; a[1]  = _01; a[2]  = _02; a[3]  = _03;
		a[4]  = _10; a[5]  = _11; a[6]  = _12; a[7]  = _13;
		a[8]  = _20; a[9]  = _21; a[10] = _22; a[11] = _23;
		a[12] = _30; a[13] = _31; a[14] = _32; a[15] = _33;
#endif
	}

	explicit mat4(const mat3<T>& m)
	{
		for(uint8_t i = 0; i < 3; ++i)
		{
			for(uint8_t j = 0; j < 3; ++j)
				a[4 * i + j] = m.a[3 * i + j];
			a[4 * i + 3] = 0;
		}
		
		a[12] = a[13] = a[14] = 0;
		a[15] = 1;
	}
	
	explicit mat4(const mat3<T>& basis, const vec3<T>& origin)
	{
		v[0] = vec4<T>(basis[0].x, basis[0].y, basis[0].z, T(0));
		v[1] = vec4<T>(basis[1].x, basis[1].y, basis[1].z, T(0));
		v[2] = vec4<T>(basis[2].x, basis[2].y, basis[2].z, T(0));
		v[3] = vec4<T>(  origin.x,   origin.y,   origin.z, T(1));
	}

	/**
	 * @brief retrieve the nth column in a COLUMN_MAJOR matrix, or the nth row in a ROW_MAJOR matrix
	 */
	const vec4<T>& operator [](size_t n) const { return v[n]; }
	      vec4<T>& operator [](size_t n)       { return v[n]; }

	T* data()             noexcept { return a; }
	const T* data() const noexcept { return a; }

	// assignment operators
	mat4<T>& operator +=(const mat4<T>& M)
	{
		for(int8_t i = 0; i < (ORDER * ORDER); ++i)
			a[i] += M.a[i];
		return *this;
	}

	mat4<T>& operator -=(const mat4<T>& M)
	{
		for(int8_t i = 0; i < (ORDER * ORDER); ++i)
			a[i] -= M.a[i];

		return *this;
	}

	mat4<T>& operator *=(const mat4<T>& m)
	{
		mat4<T> product = *this * m;
		*this = product;
		return *this;
	}

	mat4<T>& operator /=(const mat4<T>& m)
	{
		mat4<T> quotient = *this / m;
		*this = quotient;
		return *this;
	}

	mat4<T>& operator *=(T scalar)
	{
		for(int8_t i = 0; i < (ORDER * ORDER); ++i)
			a[i] *= scalar;
		return *this;
	}

	mat4<T>& operator /=(T scalar)
	{
		for(int8_t i = 0; i < (ORDER * ORDER); ++i)
			a[i] /= scalar;
		return *this;
	}

	// unary operators
	mat4<T> operator +() const { return *this; }
	mat4<T> operator -() const
	{
		mat4<T> result;
		for(int8_t i = 0; i < (ORDER * ORDER); ++i)
			result.a[i] = -a[i];
		return result;
	}

	friend bool operator ==(const mat4<T>& lhs, const mat4<T>& rhs)
	{
		for(int8_t i = 0; i < (ORDER * ORDER); ++i)
			if(!fuzzyEqual(lhs.a[i], rhs.a[i]))
				return false;
		
		return true;
	}
	
	friend bool operator !=(const mat4<T>& lhs, const mat4<T>& rhs)
	{
		return !(lhs == rhs);
	}
	
	// binary operators
	friend mat4<T> operator +(const mat4<T>& lhs, const mat4<T>& rhs)
	{
		mat4<T> sum;
		for(int8_t i = 0; i < (ORDER * ORDER); ++i)
			sum.a[i] = lhs.a[i] + rhs.a[i];
		return sum;
	}
	
	friend mat4<T> operator -(const mat4<T>& lhs, const mat4<T>& rhs)
	{
		mat4<T> difference;
		for(int8_t i = 0; i < (ORDER * ORDER); ++i)
			difference.a[i] = lhs.a[i] - rhs.a[i];
		return difference;
	}
	
	friend mat4<T> operator *(const mat4<T>& lhs, const mat4<T>& rhs)
	{
		mat4<T> product(T(0));

		// multiply order is i-k-j, not i-j-k.
		for(int8_t i = 0; i < ORDER; ++i)
			for(int8_t k = 0; k < ORDER; ++k)
				for(int8_t j = 0; j < ORDER; ++j)
					product[i][j] += lhs[k][j] * rhs[i][k];

		return product;
	}
	
	friend mat4<T> operator /(const mat4<T>& lhs, const mat4<T>& rhs)
	{
		return lhs * rhs.inverse();
	}

	friend mat4<T> operator *(const mat4<T>& lhs, T value)
	{
		mat4<T> result;
		for(int8_t i = 0; i < (ORDER * ORDER); ++i)
			result.a[i] = lhs.a[i] * value;
		return result;
	}
//	mat4<T> operator /(T scalar) const;
//	friend mat4<T> operator *(T, const mat4<T>& M);

	/**
	 * In row major, we use left mulplication.
	 * @param[in] row row vector
	 * @return vector * matrix
	 */
	vec4<T> leftMultiply(const vec4<T>& row) const
	{
		vec4<T> vm;
		vm[0] = row[0] * v[0][0] + row[1] * v[0][1] + row[2] * v[0][2] + row[3] * v[0][3];
		vm[1] = row[0] * v[1][0] + row[1] * v[1][1] + row[2] * v[1][2] + row[3] * v[1][3];
		vm[2] = row[0] * v[2][0] + row[1] * v[2][1] + row[2] * v[2][2] + row[3] * v[2][3];
		vm[3] = row[0] * v[3][0] + row[1] * v[3][1] + row[2] * v[3][2] + row[3] * v[3][3];
		return vm;
	}
	
	/**
	 * In column major, we use right mulplication.
	 * @param[in] column column vector
	 * @return matrix * vector.
	 */
	vec4<T> rightMultiply(const vec4<T>& column) const
	{
		vec4<T> mv;
		mv[0] = v[0][0] * column[0] + v[1][0] * column[1] + v[2][0] * column[2] + v[3][0] * column[3];
		mv[1] = v[0][1] * column[0] + v[1][1] * column[1] + v[2][1] * column[2] + v[3][1] * column[3];
		mv[2] = v[0][2] * column[0] + v[1][2] * column[1] + v[2][2] * column[2] + v[3][2] * column[3];
		mv[3] = v[0][3] * column[0] + v[1][3] * column[1] + v[2][3] * column[2] + v[3][3] * column[3];
		return mv;
	}
	
#if COLUMN_MAJOR
	friend vec4<T> operator *(const mat4<T>& m, const vec4<T>& v) { return m.rightMultiply(v); }
#else
	friend vec4<T> operator *(const vec4<T>& v, const mat4<T>& m) { return m.leftMultiply(v);  }
#endif
	
	template <typename charT, class traits>
	friend std::basic_istream<charT, traits>& operator >>(
			std::basic_istream<charT, traits> &is, mat4<T> &M)
	{
		// TODO
		assert(false);
		(void)M;
		return is;
	}

	template <typename U, typename charT, class traits>
	friend std::basic_ostream<charT, traits>& operator <<(
			std::basic_ostream<charT, traits> &os, const mat4<U> &M)
	{
		std::basic_ostringstream<charT, traits> stream;
		stream.flags(os.flags());
		stream.imbue(os.getloc());
		stream.precision(os.precision());

		for(int8_t i = 0; i < ORDER; ++i)
		{
			stream << '[' << ' ';
			for(int8_t j = 0; j < ORDER; ++j)
				stream << std::setw(12) << std::setfill(' ') << M[j][i] << ' ';
			stream << ']' << '\n';
		}
		return os << stream.str();
	}

	void assign(T value)
	{
		for(int8_t i = 0; i < (ORDER * ORDER); ++i)
			a[i] = value;
	}

	void setIdentity()
	{
		for(int8_t i = 1; i < 14; ++i)
			a[i] = static_cast<T>(0);
		a[15] = a[10] = a[5] = a[0] = static_cast<T>(1);
	}

	/**
	 * @brief Function turns a row major matrix to a column major matrix, or vice versa.
	 */
	void transpose()
	{
/*
	column major order            row major order
	[_0, _4, _8 , _12]          [_0 , _1 , _2 , _3 ]
	[_1, _5, _9 , _13]          [_4 , _5 , _6 , _7 ]
	[_2, _6, _10, _14]          [_8 , _9 , _10, _11]
	[_3, _7, _11, _15]          [_12, _13, _14, _15]
*/
#if 0
		for(int32_t i = 0; i < ORDER; ++i)
		for(int32_t j = i + 1; j < ORDER; ++j)
			std::swap(v[i][j], v[j][i]);
#else
		std::swap(a[ 1], a[ 4]);
		std::swap(a[ 2], a[ 8]);
		std::swap(a[ 3], a[12]);
		std::swap(a[ 6], a[ 9]);
		std::swap(a[ 7], a[13]);
		std::swap(a[11], a[14]);
#endif
	}

	mat4<T> inverse() const
	{
		// 'determinant' only accept floating-point inputs
		static_assert(std::is_floating_point<T>::value, "limited to floating-point type only");

		const T det = determinant();
		if(isZero<T>(det))  // Singular matrix
		{
			// Setting all elements to NAN is not correct in a mathematical sense,
			// but it's easy to debug errors.
			return mat4<T>(std::numeric_limits<T>::quiet_NaN());
		}

		// TODO: optimization
		mat4<T> M;
		M.a[0] = +mat3<T>(a[5], a[9], a[13],  a[6], a[10], a[14],  a[7], a[11], a[15]).determinant();
		M.a[1] = -mat3<T>(a[1], a[9], a[13],  a[2], a[10], a[14],  a[3], a[11], a[15]).determinant();
		M.a[2] = +mat3<T>(a[1], a[5], a[13],  a[2], a[ 6], a[14],  a[3], a[ 7], a[15]).determinant();
		M.a[3] = -mat3<T>(a[1], a[5], a[ 9],  a[2], a[ 6], a[10],  a[3], a[ 7], a[11]).determinant();

		M.a[4] = -mat3<T>(a[4], a[8], a[12],  a[6], a[10], a[14],  a[7], a[11], a[15]).determinant();
		M.a[5] = +mat3<T>(a[0], a[8], a[12],  a[2], a[10], a[14],  a[3], a[11], a[15]).determinant();
		M.a[6] = -mat3<T>(a[0], a[4], a[12],  a[2], a[ 6], a[14],  a[3], a[ 7], a[15]).determinant();
		M.a[7] = +mat3<T>(a[0], a[4], a[ 8],  a[2], a[ 6], a[10],  a[3], a[ 7], a[11]).determinant();

		M.a[8] = +mat3<T>(a[4], a[8], a[12],  a[5], a[ 9], a[13],  a[7], a[11], a[15]).determinant();
		M.a[9] = -mat3<T>(a[0], a[8], a[12],  a[1], a[ 9], a[13],  a[3], a[11], a[15]).determinant();
		M.a[10]= +mat3<T>(a[0], a[4], a[12],  a[1], a[ 5], a[13],  a[3], a[ 7], a[15]).determinant();
		M.a[11]= -mat3<T>(a[0], a[4], a[ 8],  a[1], a[ 5], a[ 9],  a[3], a[ 7], a[11]).determinant();

		M.a[12]= -mat3<T>(a[4], a[8], a[12],  a[5], a[ 9], a[13],  a[6], a[10], a[14]).determinant();
		M.a[13]= +mat3<T>(a[0], a[8], a[12],  a[1], a[ 9], a[13],  a[2], a[10], a[14]).determinant();
		M.a[14]= -mat3<T>(a[0], a[4], a[12],  a[1], a[ 5], a[13],  a[2], a[ 6], a[14]).determinant();
		M.a[15]= +mat3<T>(a[0], a[4], a[ 8],  a[1], a[ 5], a[ 9],  a[2], a[ 6], a[10]).determinant();

		M /= det;
		return M;
	}

	T determinant() const
	{
/*
		mat3<T> A0(a[5], a[6], a[7], a[9], a[10], a[11], a[13], a[14], a[15]);
		mat3<T> A1(a[4], a[6], a[7], a[8], a[10], a[11], a[12], a[14], a[15]);
		mat3<T> A2(a[4], a[5], a[7], a[8], a[9],  a[11], a[12], a[13], a[15]);
		mat3<T> A3(a[4], a[5], a[6], a[8], a[9],  a[10], a[12], a[13], a[14]);
		return a[0]*A0.det() - a[1]*A1.det() + a[2]*A2.det() - a[3]*A3.det();
*/
		// determinant doesn't care whether the matrix layout is row-major or column-major.
		return
			+ a[0]*(a[5]*(a[10]*a[15] - a[11]*a[14]) + a[6]*(a[11]*a[13] - a[9]*a[15]) + a[7]*(a[9]*a[14] - a[10]*a[13]))
			- a[1]*(a[4]*(a[10]*a[15] - a[11]*a[14]) + a[6]*(a[11]*a[12] - a[8]*a[15]) + a[7]*(a[8]*a[14] - a[10]*a[12]))
			+ a[2]*(a[5]*(a[11]*a[12] - a[ 8]*a[15]) + a[7]*(a[ 8]*a[13] - a[9]*a[12]) + a[4]*(a[9]*a[15] - a[11]*a[13]))
			- a[3]*(a[4]*(a[ 9]*a[14] - a[10]*a[13]) + a[5]*(a[10]*a[12] - a[8]*a[14]) + a[6]*(a[8]*a[13] - a[ 9]*a[12]));
	}

	mat4<T>& translate(const vec3<T>& v)
	{
		a[12] += v.x;
		a[13] += v.y;
		a[14] += v.z;
		return *this;
	}

	static mat4<T> getRotationX(T angle)
	{
		const T c = std::cos(angle), s = std::sin(angle);
		return mat4<T>(
			1, 0, 0, 0,
			0, c,-s, 0,
			0, s, c, 0,
			0, 0, 0, 1);
	}
	
	mat4<T>& rotateX(T angle)
	{
		const T c = std::cos(angle), s = std::sin(angle);
		
		T tmp= s*a[1] + c*a[2];
		a[1] = c*a[1] - s*a[2];
		a[2] = tmp;

		tmp  = s*a[5] + c*a[6];
		a[5] = c*a[5] - s*a[6];
		a[6] = tmp;

		tmp  = s*a[9] + c*a[10];
		a[9] = c*a[9] - s*a[10];
		a[10]= tmp;

		tmp  = s*a[13] + c*a[14];
		a[13]= c*a[13] - s*a[14];
		a[14]= tmp;

		return *this;
	}

	static mat4<T> getRotationY(T angle)
	{
		const T c = std::cos(angle), s = std::sin(angle);
		return mat4<T>(
			 c, 0, s, 0,
			 0, 1, 0, 0,
			-s, 0, c, 0,
			 0, 0, 0, 1);
	}
	
	mat4<T> rotateY(T angle)
	{
		const T c = std::cos(angle), s = std::sin(angle);
		
		T tmp= c*a[2] - s*a[0];
		a[0] = s*a[2] + c*a[0];
		a[2] = tmp;

		tmp  = c*a[6] - s*a[4];
		a[4] = s*a[6] + c*a[4];
		a[6] = tmp;

		tmp  = c*a[10] - s*a[8];
		a[8] = s*a[10] + c*a[8];
		a[10]= tmp;

		tmp  = c*a[14] - s*a[12];
		a[12]= s*a[14] + c*a[12];
		a[14]= tmp;

		return *this;
	}

	static mat4<T> getRotationZ(T angle)
	{
/*
		 * [_0, _4, _8 , _12]          [_0 , _1 , _2 , _3 ]
		 * [_1, _5, _9 , _13]          [_4 , _5 , _6 , _7 ]
		 * [_2, _6, _10, _14]          [_8 , _9 , _10, _11]
		 * [_3, _7, _11, _15]          [_12, _13, _14, _15]
*/
		const T c = std::cos(angle), s = std::sin(angle);
		return mat4<T>(
			c,-s, 0, 0,
			s, c, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
	}
	
	mat4<T>& rotateZ(T angle)
	{
		const T c = std::cos(angle), s = std::sin(angle);
		
		T tmp= s*a[0] + c*a[1];
		a[0] = c*a[0] - s*a[1];
		a[1] = tmp;

		tmp  = s*a[4] + c*a[5];
		a[4] = c*a[4] - s*a[5];
		a[5] = tmp;

		tmp  = s*a[8] + c*a[9];
		a[8] = c*a[8] - s*a[9];
		a[9] = tmp;

		tmp  = s*a[12] + c*a[13];
		a[12]= c*a[12] - s*a[13];
		a[13]= tmp;
		return *this;
	}

	static mat4<T> getRotation(const vec3<T>& normal, const T& angle)
	{
		assert(fuzzyEqual(static_cast<T>(1), normal.length()));
		const T &x = normal.x, &y = normal.y, &z = normal.z;

		const T c = std::cos(angle), C = 1 - c, s = std::sin(angle);
		return mat4<T>(
			   c + x*x*C, -z*s + x*y*C,  y*s + x*z*C, 0,
			 z*s + y*x*C,    c + y*y*C, -x*s + y*z*C, 0,
			-y*s + z*x*C,  x*s + z*y*C,    c + z*z*C, 0,
			           0,            0,            0, 1);
	}

	mat4<T>& scale(const vec3<T>& v)
	{
		// TODO: row major & column major
		a[0] *= v.x; a[4] *= v.x; a[8]  *= v.x; a[12] *= v.x;
		a[1] *= v.y; a[5] *= v.y; a[9]  *= v.y; a[13] *= v.y;
		a[2] *= v.z; a[6] *= v.z; a[10] *= v.z; a[14] *= v.z;
//		a[3] *= 1.0; a[7] *= 1.0; a[11] *= 1.0; a[15] *= 1.0;
		return *this;
	}
	
	mat4<T>& scale(const T& s)
	{
		a[0] *= s; a[4] *= s; a[8]  *= s; a[12] *= s;
		a[1] *= s; a[5] *= s; a[9]  *= s; a[13] *= s;
		a[2] *= s; a[6] *= s; a[10] *= s; a[14] *= s;
		return *this;
	}

};

using mat4f = mat4<float> ;
using mat4d = mat4<double>;

}  // namespace pea
#endif  // PEA_MATH_MAT4_H_
