#ifndef PEA_UTIL_TYPE_TRAIT_H_
#define PEA_UTIL_TYPE_TRAIT_H_

#include <type_traits>

namespace pea {

/**
 * A template dependent false, usually used with static_assert.
 */
template<class T>
struct dependent_false: std::false_type {};

template<typename T>
struct is_vector: public std::false_type {};

template<typename T, typename A>
struct is_vector<std::vector<T, A>>: public std::true_type {};

}  // namespace pea
#endif  // PEA_UTIL_TYPE_TRAIT_H_
