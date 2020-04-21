#ifndef PEA_MATH_HASH_H_
#define PEA_MATH_HASH_H_

#include "math/vec2.h"
#include "math/vec3.h"
#include "math/vec4.h"


namespace pea {

// The Hash http://burtleburtle.net/bob/hash/doobs.html
/**
 * https://en.wikipedia.org/wiki/Fowler�CNoll�CVo_hash_function
 */
class FNV1a
{
private:
	static constexpr bool is32Bit = sizeof(void*) <= 4;  //   32 bit:                64 bit;
	static const std::size_t fnv_prime        = is32Bit?   16777619u:        1099511628211u;
	static const std::size_t fnv_offset_basis = is32Bit? 2166136261u: 14695981039346656037u;
	
	// For 128 bit machines:
//	static const std::size_t fnv_prime = 309485009821345068724781401u;
//	static const std::size_t fnv_offset_basis = 275519064689413815358837431229664493455u;

	// For 256 bit machines:
//	static const std::size_t fnv_prime = 374144419156711147060143317175368453031918731002211u;
//	static const std::size_t fnv_offset_basis = 100029257958052580907070968620625704837092796014241193945225284501741471925557u;
	
public:
	std::size_t operator()(const void* key, std::size_t len) noexcept
	{
		std::size_t state = fnv_offset_basis;
		for(const uint8_t* p = static_cast<const uint8_t*>(key), *end = p + len; p < end; ++p)
			state = (state ^ *p) * fnv_prime;

		return state;
	}

};

// http://www.boost.org/doc/libs/1_55_0/doc/html/hash/combine.html
// http://stackoverflow.com/questions/4948780/magic-number-in-boosthash-combine
// phi = (1 + sqrt(5)) / 2;  2^32 / phi = 0x9e3779b9;
// Here is a proposal N3797 on Hashing tuple-like types,
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3983.pdf
// Boost also come up with a hash_combine function
// http://www.boost.org/doc/libs/1_53_0/doc/html/hash/reference.html#boost.hash_combine
template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

}  // namespace pea

/*
	Extending std namespace is allowed in several situtations.
	See this thread http://en.cppreference.com/w/cpp/language/extending_std for details.
	http://stackoverflow.com/questions/8513417/what-can-and-cant-i-specialize-in-the-std-namespace
	http://stackoverflow.com/questions/24361884/how-to-specialize-stdhasht-for-user-defined-types
	http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3980.html and N3333
*/

namespace std
{
	template<typename T>
	struct hash<pea::vec2<T>>
	{
		size_t operator()(const pea::vec2<T>& v) const noexcept
		{
#if 0
			std::size_t seed = 0;
			boost::hash_combine(seed, v.x);
			boost::hash_combine(seed, v.y);
			return seed;
#else
			pea::FNV1a fnv1a;
			return fnv1a(&v, sizeof(v));
#endif
		}
	};

	template<typename T>
	struct hash<pea::vec3<T>>
	{
		size_t operator()(const pea::vec3<T>& v) const noexcept
		{
			pea::FNV1a fnv1a;
			return fnv1a(&v, sizeof(v));
		}	
	};
	
	template<typename T>
	struct hash<pea::vec4<T>>
	{
		size_t operator()(const pea::vec4<T>& v) const noexcept
		{
			pea::FNV1a fnv1a;
			return fnv1a(&v, sizeof(v));
		}
	};
	
	
}  // namespace std
#endif  // PEA_MATH_HASH_H_
