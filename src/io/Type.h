#ifndef PEA_IO_TYPE_H_
#define PEA_IO_TYPE_H_

#include <cstdint>
#include <string>
#include <vector>

#include "math/vec2.h"
#include "math/vec3.h"
#include "math/vec4.h"

namespace pea {

class Type final
{
public:
	Type() = delete;
	~Type() = delete;

public:
	enum _: std::uint8_t
	{
		TYPE_UNKNOWN = 0,
		
		TYPE_BOOL,
		
		TYPE_INT8,
		TYPE_INT16,
		TYPE_INT32,
		TYPE_INT64,
		
		TYPE_UINT8,
		TYPE_UINT16,
		TYPE_UINT32,
		TYPE_UINT64,
		
		TYPE_FLOAT,
		TYPE_DOUBLE,
		
		TYPE_STRING,

		TYPE_VEC2I,
		TYPE_VEC2F,
		TYPE_VEC3I,
		TYPE_VEC3F,
		TYPE_VEC4I,
		TYPE_VEC4F,

		COLOR,
	};
/*
	class String
	{
	public:
		uint32_t size;
		const char* ptr;
	public:
		String(uint32_t size, const char* ptr) { this->size=size; this->ptr=ptr; }
		String(const std::string& str) { this->size=str.size(); this->ptr=str.c_str(); }
		String(const String& other) { this->size=other.size; this->ptr=other.ptr; }
	};
*/
/*
	struct Value
	{
	private:
		Type type;
		union _
		{
			bool     boolean;
			int32_t  i32;
			uint32_t u32;
			int64_t  i64;
			uint64_t u64;
			float    f32;
			double   f64;
			String   str;
		} value;
	public:
		explicit Value(bool v)     { type=BOOLEAN; value.boolean=v;}
		explicit Value(int32_t v)  { type=INTEGER; value.i32=v;    }
		explicit Value(uint32_t v) { type=INTEGER; value.u32=v;    }
		explicit Value(float v)    { type=FLOAT;   value.f32=v;    }
		explicit Value(double v)   { type=DOUBLE;  value.f64=v;    }
		explicit Value(String v)   { type=DOUBLE;  value.str=v;    }
	};

private:
	const std::string key;
	Value             value;
*/
//	static const char* BLANK;

public:
	static int32_t parseInt(const char* &token);
	static uint32_t parseUint(const char* &token);
	static float parseFloat(const char* &token);

	/**
	 * parse triples: i, i/j/k, i//k, i/j, used in .OBJ file.
	 */
	static vec3i parseInt3(const char* &token);
	static vec3u parseUint3(const char* &token);

	static vec2f parseFloat2(const char* &token);
	static vec3f parseFloat3(const char* &token);
	static vec4f parseFloat4(const char* &token);

	/**
	 * trim blank space(" \t")
	 */
	static std::string parseString(const char* &token);

	/**
	 * check whether string @code{tag} begins with @code{str}
	 */
	inline static bool beginWith(const char* str, const char* tag);
	inline static bool beginWith(const char* str, const char* tag, size_t len);

	/**
	 * C++ lack of std::string split() function, see the proposal
	 * {@link http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3593.html}
	 * C has strtok function, but it has its disadvantages. This function modify their
	 * first argument, and it cannot be used on constant strings.
	 */
	static std::vector<std::string> split(const char* str, const char* delim);

	/**
	 * @brief Splits a string into substrings separated by delimiter, and returns a
	 *  std::vector of all those substrings. E.g.:
	 *  split("a b=c d=e", ' ')  --> ["a", "b=c", "d=e"]
	 *
	 * @param str The string to split.
	 * @param delim The delimiter that is used to split string.
	 * @return split substrings
	 */
	static std::vector<std::string> split(const char* str, char delim);

	static std::vector<std::string> split(const char* str);

	template<typename T>
	void set(const std::string& key, T value);

};

}  // namespace pea
#endif  // PEA_IO_TYPE_H_
