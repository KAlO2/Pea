#ifndef PEA_MATH_MAT3_H_
#define PEA_MATH_MAT3_H_

#include "pea/config.h"
#include "math/vec2.h"
#include "math/vec3.h"

namespace pea {

template <typename T>
class mat3
{
public:
	union
	{
		vec3<T> v[3];
		T a[9];
	};

	typedef T value_type;
	static constexpr int8_t ORDER = 3;
//	static constexpr mat3<T> ONE = mat3<T>(T(1));

public:
	mat3(): mat3(static_cast<T>(1)) {}
	~mat3() = default;
	mat3(const mat3<T>& M) = default;
	mat3<T>& operator =(const mat3<T>& rhs) = default;

	// The memory areas must not overlap. Use memmove if the memory areas do overlap.
	explicit mat3(const T* array)
	{
//		memcpy(a, array, sizeof(a));
		for(int8_t i = 0; i < (ORDER * ORDER); ++i)
			a[i] = array[i];
	}

	explicit mat3(T diagonal): mat3(diagonal, diagonal, diagonal) {}

	mat3(T d0, T d1, T d2)
	{
		for(int8_t i = 1; i < 8; ++i)
			a[i] = static_cast<T>(0);
		a[0] = d0;
		a[4] = d1;
		a[8] = d2;
	}

	mat3(const vec3<T> &v0, const vec3<T> &v1, const vec3<T> &v2)
	{
		v[0] = v0;
		v[1] = v1;
		v[2] = v2;
	}
	
	mat3(T _00, T _01, T _02,
	     T _10, T _11, T _12,
	     T _20, T _21, T _22)
	{
#if COLUMN_MAJOR
		a[0] = _00; a[3] = _01; a[6] = _02;
		a[1] = _10; a[4] = _11; a[7] = _12;
		a[2] = _20; a[5] = _21; a[8] = _22;
#else
		a[0] = _00; a[1] = _01; a[2] = _02;
		a[3] = _10; a[4] = _11; a[5] = _12;
		a[6] = _20; a[7] = _21; a[8] = _22;
#endif
	}

	/**
	 * @brief retrieve the nth column in a COLUMN_MAJOR matrix, or the nth row in a ROW_MAJOR matrix
	 */
	const vec3<T>& operator [](size_t n) const { return v[n]; }
	      vec3<T>& operator [](size_t n)       { return v[n]; }

	T* data()             noexcept { return a; }
	const T* data() const noexcept { return a; }

	// assignment operators
	mat3<T>& operator +=(const mat3<T>& M)
	{
		for(int8_t i = 0; i < (ORDER * ORDER); ++i)
			a[i] += M.a[i];
		return *this;
	}

	mat3<T>& operator -=(const mat3<T>& M)
	{
		for(int8_t i = 0; i < (ORDER * ORDER); ++i)
			a[i] -= M.a[i];
		return *this;
	}

	/**
	 * @brief matrix multiplication
	 * Note that matrix multiplication is not commutative, namely M1*M2 != M2*M1.
	 * operator *= means multiplying the value of the first operand by the value of the
	 * second operand; store the result in the object specified by the first operand.
	 * so M1 *= M2 is equivalent to M1 = M1 * M2;
	 */
	mat3<T>& operator *=(const mat3<T>& M)
	{
		mat3<T> L(*this);
		this->assign(T(0));
		
		// multiply order is i-k-j, not i-j-k.
		for(int8_t i = 0; i < ORDER; ++i)
		{
			for(int8_t k = 0; k < ORDER; ++k)
			{
				T L_i_k = L[i][k];
				for(int8_t j = 0; j < ORDER; ++j)
					(*this)[i][j] += L_i_k * M[k][j];
			}
		}
		return *this;
	}

	mat3<T>& operator /=(const mat3<T>& M)
	{
		mat3<T> invM = M.inverse();
		mat3<T> L = *this;
		*this = L * invM;
		return *this;
	}

	mat3<T>& operator *=(T scalar)
	{
		for(int8_t i = 0; i < (ORDER * ORDER); ++i)
			a[i] *= scalar;
		return *this;
	}

	mat3<T>& operator /=(T scalar)
	{
		for(int8_t i = 0; i < (ORDER * ORDER); ++i)
			a[i] /= scalar;
		return *this;
	}

	// unary operators
	mat3<T> operator +() const { return *this; }
	mat3<T> operator -() const
	{
		mat3<T> result;
		for(int8_t i = 0; i < (ORDER * ORDER); ++i)
			result.a[i] = -a[i];
		return result;
	}

	friend bool operator ==(const mat3<T>& lhs, const mat3<T>& rhs)
	{
		for(int8_t i = 0; i < (ORDER * ORDER); ++i)
			if(!fuzzyEqual(lhs.a[i], rhs.a[i]))
				return false;
		
		return true;
	}
	
	friend bool operator !=(const mat3<T>& lhs, const mat3<T>& rhs)
	{
		return !(lhs == rhs);
	}

	/**
	 * binary operators, add subtract, multiply and divide operations for scalar
	 * @{
	 */
	friend mat3<T> operator +(const mat3<T>& lhs, const mat3<T>& rhs) { mat3<T> result(lhs); result += rhs; return result; }
	friend mat3<T> operator -(const mat3<T>& lhs, const mat3<T>& rhs) { mat3<T> result(lhs); result -= rhs; return result; }
	friend mat3<T> operator *(const mat3<T>& lhs, const mat3<T>& rhs) { mat3<T> result(lhs); result *= rhs; return result; }
	friend mat3<T> operator /(const mat3<T>& lhs, const mat3<T>& rhs) { mat3<T> result(lhs); result /= rhs; return result; }

	friend mat3<T> operator *(const mat3<T>& lhs, T value)
	{
		mat3<T> result;
		for(int8_t i = 0; i < (ORDER * ORDER); ++i)
			result.a[i] = lhs.a[i] * value;
		return result;
	}

	friend mat3<T> operator *(T value, const mat3<T>& rhs)
	{
		mat3<T> result;
		for(int8_t i = 0; i < (ORDER * ORDER); ++i)
			result.a[i] = value * rhs.a[i];
		return result;
	}

	friend mat3<T> operator /(const mat3<T>& lhs, T value)
	{
		mat3<T> result;
		for(int8_t i = 0; i < (ORDER * ORDER); ++i)
			result.a[i] = lhs.a[i] / value;
		return result;
	}
	/** @} */

	/**
	 * In row major, we use left mulplication.
	 * @param[in] row row vector
	 * @return vector * matrix
	 */
	vec3<T> leftMultiply(const vec3<T>& row) const
	{
		vec3<T> vm;
		vm[0] = row[0] * v[0][0] + row[1] * v[0][1] + row[2] * v[0][2];
		vm[1] = row[0] * v[1][0] + row[1] * v[1][1] + row[2] * v[1][2];
		vm[2] = row[0] * v[2][0] + row[1] * v[2][1] + row[2] * v[2][2];
		return vm;
	}
	
	/**
	 * In column major, we use right mulplication.
	 * @param[in] column column vector
	 * @return matrix * vector.
	 */
	vec3<T> rightMultiply(const vec3<T>& column) const
	{
		vec3<T> mv;
		mv[0] = v[0][0] * column[0] + v[1][0] * column[1] + v[2][0] * column[2];
		mv[1] = v[0][1] * column[0] + v[1][1] * column[1] + v[2][1] * column[2];
		mv[2] = v[0][2] * column[0] + v[1][2] * column[1] + v[2][2] * column[2];
		return mv;
	}
	
#if COLUMN_MAJOR
	friend vec3<T> operator *(const mat3<T>& m, const vec3<T>& v) { return m.rightMultiply(v); }
#else
	friend vec3<T> operator *(const vec3<T>& v, const mat3<T>& m) { return m.leftMultiply(v);  }
#endif

	template <typename charT, class traits>
	friend std::basic_istream<charT, traits>& operator >>(
			std::basic_istream<charT, traits> &is, mat3<T> &M)
	{
		// TODO
		assert(false);
		(void)M;
		return is;
	}

	template <typename charT, class traits>
	friend std::basic_ostream<charT, traits>& operator <<(
			std::basic_ostream<charT, traits> &os, const mat3<T> &M)
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

	void identity()
	{
		for(int8_t i = 1; i < 8; ++i)
			a[i] = static_cast<T>(0);
		a[8] = a[4] = a[0] = static_cast<T>(1);
	}

	void transpose()
	{
		std::swap(a[1], a[3]); // m[0][1] <--> m[1][0]
		std::swap(a[2], a[6]); // m[0][2] <--> m[2][0]
		std::swap(a[5], a[7]); // m[1][2] <--> m[2][1]
	}

	bool isDiagonal() const
	{
		return
			isZero<T>(a[1]) && isZero<T>(a[2]) && isZero<T>(a[3]) &&
			isZero<T>(a[5]) && isZero<T>(a[6]) && isZero<T>(a[7]);
	}

	vec3<T> diagonal() const { return vec3<T>(a[0], a[4], a[8]); }
	
	mat3<T>& translate(const vec2<T>& v)
	{
		a[6] += v.x;
		a[7] += v.y;
		return *this;
	}
	
#if 0
	static mat3<T> rotate(T angle)
	{
		const T c = std::cos(angle), s = std::sin(angle);
		return mat3<T>(
			c,-s, 0,
			s, c, 0,
			0, 0, 1);
	}
#else
	mat3<T>& rotate(T angle)
	{
		const T c = std::cos(angle), s = std::sin(angle);
		
		// [ c,-s, 0]   [0, 3, 6]
		// [ s, c, 0] * [1, 4, 7]
		// [ 0, 0, 1]   [2, 5, 8]
		T tmp= s * a[0] + c * a[1];
		a[0] = c * a[0] - s * a[1];
		a[1] = tmp;
		
		tmp  = s * a[3] + c * a[4];
		a[3] = c * a[3] - s * a[4];
		a[4] = tmp;
		
		return *this;
	}
#endif
	mat3<T>& scale(const vec2<T>& v)
	{
		a[0] *= v.x; a[3] *= v.x; a[6] *= v.x;
		a[1] *= v.y; a[4] *= v.y; a[7] *= v.y;
//		a[2] *= 1.0; a[5] *= 1.0; a[8] *= 1.0;
		return *this;
	}

	mat3<T>& scale(const T& s)
	{
		a[0] *= s; a[3] *= s; a[6] *= s;
		a[1] *= s; a[4] *= s; a[7] *= s;
//		a[2] *= 1; a[5] *= 1; a[8] *= 1;
		return *this;
	}
	
	mat3<T>& skew(const vec2<T>& angle)
	{
		float x = std::tan(angle.x);
		float y = std::tan(angle.y);
#if 0
		mat3<T> m(1, x, 0,
				y, 1, 0,
				0, 0, 1);
		*this = m * *this;
#else
		float _0 = a[0] + x * a[1];
		float _1 = y * a[0] + a[1];
		a[0] = _0;
		a[1] = _1;
		
		float _3 = a[3] + x * a[4];
		float _4 = y * a[3] + a[4];
		a[3] = _3;
		a[4] = _4;
#endif
		return *this;
	}
	
	/**
	 * return the inverse matrix of the original matrix.
	 * A^-1 = A.adjoint() / A.det();
	 * adjoint matrix of A is cofactor matrix of A transposed.
	 */
	mat3<T> inverse() const
	{
		// 'determinant' only accept floating-point inputs
		static_assert(std::is_floating_point<T>::value, "limited to floating-point type only");

		// [0, 3, 6]
		// [1, 4, 7]
		// [2, 5, 8]
		mat3<T> adjoint;
/*
	for(int8_t j = 0 ;j < ORDER; ++j)
	{
		for(int i = 0; i < ORDER; ++i)
			printf("adjoint.a[%d] = a[%d]*a[%d] - a[%d]*a[%d];\n", j*N + i,
					((i+1)%N)*N + (j+1)%N, ((i+2)%N)*N + (j+2)%N,
					((i+1)%N)*N + (j+2)%N, ((i+2)%N)*N + (j+1)%N);
		putchar('\n');
	}
*/
		// Those lines are generated by the comment above.
		adjoint.a[0] = a[4]*a[8] - a[5]*a[7];
		adjoint.a[1] = a[7]*a[2] - a[8]*a[1];
		adjoint.a[2] = a[1]*a[5] - a[2]*a[4];

		adjoint.a[3] = a[5]*a[6] - a[3]*a[8];
		adjoint.a[4] = a[8]*a[0] - a[6]*a[2];
		adjoint.a[5] = a[2]*a[3] - a[0]*a[5];

		adjoint.a[6] = a[3]*a[7] - a[4]*a[6];
		adjoint.a[7] = a[6]*a[1] - a[7]*a[0];
		adjoint.a[8] = a[0]*a[4] - a[1]*a[3];

		const T determinant = a[0]*adjoint.a[0] + a[1]*adjoint.a[3] + a[2]*adjoint.a[6];
		if(isZero<T>(determinant))  // Singular matrix
		{
			// Setting all elements to NAN is not correct in a mathematical sense,
			// but it's easy to debug errors.
			return mat3<T>(std::numeric_limits<T>::quiet_NaN());
		}

		adjoint /= determinant;
		return adjoint;
	}

	T determinant() const
	{
		// [0, 3, 6]
		// [1, 4, 7]
		// [2, 5, 8]
		// since det(A) == det(transpose(A)), we can use 1D array here.
		// the sign of position [i][j] is 1-((i+j)%2)*2,
		// namely, positive when i+j is even, negative when i+j is odd.
		return
			+ a[0]*(a[4]*a[8] - a[7]*a[5])
			- a[1]*(a[3]*a[8] - a[6]*a[5])
			+ a[2]*(a[3]*a[7] - a[6]*a[4]);
	}

};

using mat3f = mat3<float> ;
using mat3d = mat3<double>;

}  // namespace pea
#endif  // PEA_MATH_MAT3_H_
