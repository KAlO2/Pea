#ifndef PEA_UTIL_UTILITY_H_
#define PEA_UTIL_UTILITY_H_

#include <cassert>

namespace pea {

// use UNUSED macro to silent the warning "unused variable"
// while in C++11, you can write std::ignore = var;
#define PEA_UNUSED(var)  ((void)var)

#define SAFE_DELETE(ptr)        do { delete ptr;   ptr = nullptr; } while(false)
#define SAFE_DELETE_ARRAY(ptr)  do { delete[] ptr; ptr = nullptr; } while(false)

// http://stackoverflow.com/questions/6376000/how-does-this-array-size-template-work
// https://blogs.msdn.microsoft.com/the1/2004/05/07/how-would-you-get-the-count-of-an-array-in-c-2/
//#define NELEM(array) (sizeof(array)/sizeof(array[0]))
// int array[10];
// std::extent<decltype(array)>::value;  // yet another approach, needs C++11 
template <typename T, std::size_t N>
constexpr std::size_t sizeofArray(const T (&array)[N]) noexcept
{
	(void)array;  // std::ignore = array;  // [[maybe_unused]]
	return N;
}

constexpr uint32_t makeFourCC(uint8_t ch1, uint8_t ch2, uint8_t ch3, uint8_t ch4) noexcept
{
	return ch1 | (ch2 << 8) | (ch3 << 16) | (ch4 << 24);
}


template<typename To, typename From>
constexpr To downcast(From p)
{
#ifdef NDEBUG
	return static_cast<To>(p);
#else
	To to = dynamic_cast<To>(p);
	assert(to != nullptr);
//	if(to == nullptr)
//		throw std::bad_cast();
	return to;
#endif
}



}  // namespace pea
#endif  // PEA_UTIL_UTILITY_H_
