#ifndef PEA_IO_TYPE_UTILITY_H_
#define PEA_IO_TYPE_UTILITY_H_

#include <string>
#include <vector>

#include "math/vec2.h"
#include "math/vec3.h"
#include "math/vec4.h"

namespace pea {

class TypeUtility final
{
public:
	TypeUtility() = delete;
	~TypeUtility() = delete;

public:
	static int32_t  parseInt(const char* &token);
	static uint32_t parseUint(const char* &token);
	static float    parseFloat(const char* &token);

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
#endif  // PEA_IO_TYPE_UTILITY_H_
