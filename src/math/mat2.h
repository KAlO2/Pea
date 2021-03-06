#ifndef PEA_MATH_MAT2_H_
#define PEA_MATH_MAT2_H_

#include "pea/config.h"
#include "math/vec2.h"

namespace pea {

template <typename T>
class mat2
{
public:
	union
	{
		vec2<T> v[2];
		T       a[4];
	};

	typedef T value_type;
	static constexpr int8_t ORDER = 2;
//	static constexpr mat2<T> ONE = mat2<T>(T(1));
public:
	mat2(): mat2(static_cast<T>(1)) { }
	~mat2() = default;
	mat2(const mat2<T>& M) = default;  // { memcpy(a, M.a, sizeof(a)); }
	mat2<T>& operator =(const mat2<T>& other)// = default;
	{
		const T* b = other.a;
		if(a != &b)
		{
			a[0] = b[0];
			a[1] = b[1];
			a[2] = b[2];
			a[3] = b[3];
		}
		
		return *this;
	}
	
	explicit mat2(const T* array)
	{
//		memcpy(a, array, sizeof(a));
		a[0] = array[0];  a[1] = array[1];
		a[2] = array[2];  a[3] = array[3];
	}

	explicit mat2(T diagonal)
	{
		a[0] = a[3] = diagonal;
		a[1] = a[2] = static_cast<T>(0);
	}

	mat2(const vec2<T> &v0, const vec2<T> &v1)
	{
		v[0] = v0;
		v[1] = v1;
	}

	/**
	 * construct a 2x2 matrix
	 */
	mat2(T _00, T _01,
	     T _10, T _11)
	{
		a[0] = _00;
		a[3] = _11;
#if COLUMN_MAJOR
		a[1] = _10;
		a[2] = _01;
#else
		a[1] = _01;
		a[2] = _10;
#endif
	}

	/**
	 * @brief retrieve the nth column in a COLUMN_MAJOR matrix, or the nth row in a ROW_MAJOR matrix
	 */
	const vec2<T>& operator [](size_t n) const { return v[n]; }
	      vec2<T>& operator [](size_t n)       { return v[n]; }

	T* data()             noexcept { return a; }
	const T* data() const noexcept { return a; }

	// assignment operators
	mat2<T>& operator +=(const mat2<T>& M)
	{
		a[0] += M.a[0];
		a[1] += M.a[1];
		a[2] += M.a[2];
		a[3] += M.a[3];
		return *this;
	}
	
	mat2<T>& operator -=(const mat2<T>& M)
	{
		a[0] -= M.a[0];
		a[1] -= M.a[1];
		a[2] -= M.a[2];
		a[3] -= M.a[3];
		return *this;
	}
	
	mat2<T>& operator *=(const mat2<T>& M)
	{
		mat2<T> L(*this);  // [i][j] = sum(L[i][k] * M[k][j]);
		(*this)[0][0] = L[0][0] * M[0][0] + L[0][1] * M[1][0];
		(*this)[0][1] = L[0][0] * M[0][1] + L[0][1] * M[1][1];
		(*this)[1][0] = L[1][0] * M[0][0] + L[1][1] * M[1][0];
		(*this)[1][1] = L[1][0] * M[0][1] + L[1][1] * M[1][1];
		return *this;
	}

	mat2<T>& operator /=(const mat2<T>& M) { mat2<T>R(M); R.inverse(); return *this * R; }
	mat2<T>& operator *=(T s) { a[0] *= s; a[1] *= s; a[2] *= s; a[3] *= s; return *this; }
	mat2<T>& operator /=(T s) { a[0] /= s; a[1] /= s; a[2] /= s; a[3] /= s; return *this; }

	// unary operators
	mat2<T> operator +() const { return *this; }
	mat2<T> operator -() const { return mat2<T>(-a[0], -a[1], -a[2], -a[3]); }

	friend bool operator ==(const mat2<T>& lhs, const mat2<T>& rhs)
	{
		for(int8_t i = 0; i < (ORDER * ORDER); ++i)
			if(!fuzzyEqual(lhs.a[i], rhs.a[i]))
				return false;
		
		return true;
	}
	
	friend bool operator !=(const mat2<T>& lhs, const mat2<T>& rhs)
	{
		return !(lhs == rhs);
	}

	/**
	 * binary operators, add subtract, multiply and divide operations for scalar
	 * @{
	 */
	friend mat2<T> operator +(const mat2<T>& lhs, const mat2<T>& rhs) { mat2<T> result(lhs); result += rhs; return result; }
	friend mat2<T> operator -(const mat2<T>& lhs, const mat2<T>& rhs) { mat2<T> result(lhs); result -= rhs; return result; }
	friend mat2<T> operator *(const mat2<T>& lhs, T value)
	{
		mat2<T> result(lhs);
		T* a = result.a;
		a[0] *= value;  a[1] *= value;
		a[2] *= value;  a[3] *= value;
		return result;
	}

	friend mat2<T> operator *(T value, const mat2<T>& rhs)
	{
		mat2<T> result(rhs);
		T* a = result.a;
		a[0] *= value;  a[1] *= value;
		a[2] *= value;  a[3] *= value;
		return result;
	}

	friend mat2<T> operator /(const mat2<T>& lhs, T value)
	{
		mat2<T> result(lhs);
		T* a = result.a;
		a[0] /= value;  a[1] /= value;
		a[2] /= value;  a[3] /= value;
		return result;
	}
	/** @} */

	/**
	 * In row major, we use left mulplication.
	 * @param[in] row row vector
	 * @return vector * matrix
	 */
	vec2<T> leftMultiply(const vec2<T>& row) const
	{
		vec2<T> vm;
		vm[0] = row[0] * v[0][0] + row[1] * v[0][1];
		vm[1] = row[0] * v[1][0] + row[1] * a[1][1];
		return vm;
	}
	
	/**
	 * In column major, we use right mulplication.
	 * @param[in] column column vector
	 * @return matrix * vector.
	 */
	vec2<T> rightMultiply(const vec2<T>& column) const
	{
		vec2<T> mv;
		mv[0] = v[0][0] * column[0] + v[1][0] * column[1];
		mv[1] = v[0][1] * column[0] + v[1][1] * column[1];
		return mv;
	}
	
#if COLUMN_MAJOR
	friend vec2<T> operator *(const mat2<T>& m, const vec2<T>& v) { return m.rightMultiply(v); }
#else
	friend vec2<T> operator *(const vec2<T>& v, const mat2<T>& m) { return m.leftMultiply(v);  }
#endif

	template <typename charT, class traits>
	friend std::basic_istream<charT, traits>& operator >>(
			std::basic_istream<charT, traits> &is, mat2<T>& M)
	{
		T a[4];
		charT ch0, ch1, ch2, ch3, ch4, ch5;

		// only '[_00, _01][_10, _11]' format is allowed
		is >> ch0 >> a[0] >> ch1 >> a[1] >> ch2 >> a[2] >> ch3 >> a[3] >> ch4;
		if(ch0 == '[' && ch1 == ',' && ch2 == ']' && ch3 == '[' && ch4 == ',' && ch5 == ']')
			M = mat2<T>(a);
		else
			is.setstate(std::ios_base::failbit);

		return is;
	}

	template <typename charT, class traits>
	friend std::basic_ostream<charT, traits>& operator <<(
			std::basic_ostream<charT, traits> &os, const  mat2<T> &M)
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
		a[3] = a[2] = a[1] = a[0] = value;
	}
	
	void setIdentity()
	{
		a[3] = a[0] = 1; a[2] = a[1] = 0;
	}
	
	void transpose()
	{
		std::swap(a[1],a[2]);  // m[0][1] <--> m[1][0]
	}

	mat2<T> inverse() const
	{
		// 'determinant' only accept floating-point inputs
		static_assert(std::is_floating_point<T>::value, "limited to floating-point type only");

		// A = [a, b; c, d];
		// inv(A) = [d, -b; -c, a]/det(A) = [d, -b; -c, a]/(ad-bc)

		const T det = determinant();
		assert(!isZero<T>((det))); // singular matrix

		mat2<T> result;
		result.a[0] = a[3]; result.a[1] =-a[1];
		result.a[2] =-a[2]; result.a[3] = a[0];
		result /= det;
		return result;
	}

	/**
	 * @brief determinant
	 */
	T determinant() const
	{
		// m[0][0]*m[1][1] - m[0][1]*m[1][0];
		return a[0] * a[3] - a[1] * a[2];
	}
};

using mat2f = mat2<float> ;
using mat2d = mat2<double>;

}  // namespace pea
#endif  // PEA_MATH_MAT2_H_
