#ifndef PEA_UTIL_COMPILER_H_
#define PEA_UTIL_COMPILER_H_

#include <type_traits>

namespace pea {


#if defined(__GNUC__) || defined(__clang__)
/*
struct S{ char a; int b; }
__attribute__((packed));
*/
#  define PACKED_STRUCT(declaration)  declaration __attribute__((__packed__))
#elif defined _MSC_VER
/*
#pragma pack(push, 1)
struct S{ char a; int b; };
#pragma pack(pop)
*/
#  define PACKED_STRUCT(declaration)  __pragma(pack(push, 1)) declaration __pragma(pack(pop))
#else
#  error "PACKED_STRUCT feature unavailable/unimplemented"
#endif

// C++ attribute: likely, unlikely (since C++20)
// https://en.cppreference.com/w/cpp/language/attributes/likely
#ifdef __GNUC__
#  define LIKELY(cond)    __builtin_expect(!!(cond), 1)
#  define UNLIKELY(cond)  __builtin_expect(!!(cond), 0)
#else
#  define LIKELY(cond)    (cond)
#  define UNLIKELY(cond)  (cond)
//#define IF_UNLIKELY(cond) if (!(cond)); else
#endif


// __func__ macro goes into C++11 standard
// https://msdn.microsoft.com/en-us/library/b0084kay(v=vs.71).aspx
// but on VS <= 2013, it emits an error C2065: '__func__' : undeclared identifier
#ifdef _MSC_VER
#  if _MSC_VER < 1900
#    define __func__ __FUNCTION__
#  endif
#  define __PRETTY_FUNCTION__ __FUNCSIG__
#endif


#ifdef _MSC_VER

#ifndef stricmp
#define stricmp strcasecmp
#endif

#ifndef strnicmp
#define strnicmp strncasecmp
#endif

#ifndef sprintf
#define sprintf _sprintf
#endif

/*
	Microsoft has finally implemented snprintf in VS2015 (_MSC_VER == 1900).

	Releases prior to Visual Studio 2015 didn't have a conformant implementation.
	There are instead non-standard extensions such as _snprintf() (which doesn't
	write null-terminator on overflow) and _snprintf_s() (which can enforce
	null-termination, but returns -1 on overflow instead of the number of
	characters that would have been written).
*/
#if _MSC_VER < 1900

#define snprintf c99_snprintf
#define vsnprintf c99_vsnprintf

inline int c99_vsnprintf(char *outBuf, size_t size, const char *format, va_list ap)
{
	int count = -1;

	if (size != 0)
		count = _vsnprintf_s(outBuf, size, _TRUNCATE, format, ap);
	if (count == -1)
		count = _vscprintf(format, ap);

	return count;
}

inline int c99_snprintf(char *outBuf, size_t size, const char *format, ...)
{
	int count;
	va_list ap;

	va_start(ap, format);
	count = c99_vsnprintf(outBuf, size, format, ap);
	va_end(ap);

	return count;
}

#endif  // _MSC_VER < 1900
#endif  // _MSC_VER

template<typename E>
using is_scoped_enum = std::integral_constant<
	bool,
	std::is_enum<E>::value && !std::is_convertible<E, int>::value>;

// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1682r0.html
template<typename E>
constexpr auto underlying_cast(E e) noexcept  // -> typename std::underlying_type<E>::type
{
	static_assert(std::is_enum<E>::value, "type E must be enumeration");
	using underlying_type_t = typename std::underlying_type<E>::type;  // since C++14
	return static_cast<underlying_type_t>(e);
}

// Scoped enumeration is good. But sometimes, we only need the scope feature, without losing
// bit operations in the old days. Here comes BIT_OPERATION_FOR_SCOPED_ENUM.
#define BIT_OPERATIONS_FOR_SCOPED_ENUM(T)                                        \
	inline T operator &(const T& lhs, const T& rhs)                              \
	{                                                                            \
		using type = std::underlying_type<T>::type;                              \
		return static_cast<T>(static_cast<type>(lhs) & static_cast<type>(rhs));  \
	}                                                                            \
	                                                                             \
	inline T operator |(const T& lhs, const T& rhs)                              \
	{                                                                            \
	    static_assert(is_scoped_enum<T>::value, "T must be a scoped enum");      \
	    using type = std::underlying_type<T>::type;                              \
	    return static_cast<T>(static_cast<type>(lhs) | static_cast<type>(rhs));  \
	}                                                                            \
	                                                                             \
	inline T operator ~(const T& flag)                                           \
	{                                                                            \
		using type = std::underlying_type<T>::type;                              \
		return static_cast<T>(~static_cast<type>(flag));                         \
	}                                                                            \
	                                                                             \
	inline T& operator |=(T& lhs, const T& rhs)                                  \
	{                                                                            \
	    lhs = lhs | rhs;                                                         \
	    return lhs;                                                              \
	}                                                                            \
	                                                                             \
	inline T& operator &=(T& lhs, const T& rhs)                                  \
	{                                                                            \
	    lhs = lhs & rhs;                                                         \
	    return lhs;                                                              \
	}

}  // namespace pea
#endif  // PEA_UTIL_COMPILER_H_
