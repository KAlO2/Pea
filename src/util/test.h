#ifndef PEA_UTIL_TEST_H_
#define PEA_UTIL_TEST_H_

#include <iostream>

#include "util/Log.h"

#define LOG_MEET(expected, OP, actual)
#define LOG_FAIL(expected, OP, actual)

#define REQUIRE(condition)                                           \
	do {                                                             \
		if(!(condition))                                             \
			std::cerr << #condition << " doesn't meet "              \
					<< '@' << __FILE__ << "#L" << __LINE__ << '\n';  \
	} while(false)                                                   \

#define REQUIRE_OP(expected, OP, actual)                              \
	do {                                                              \
	    if(!((expected) OP (actual)))                                 \
	        std::cerr << #expected << " expect " << expected << ", "  \
	                << #actual << " get " << actual << " "            \
	                << '@' << __FILE__ << "#L" << __LINE__ << '\n';   \
	} while(false)                                                    \

#define REQUIRE_EQ(expected, actual) REQUIRE_OP(expected, ==, actual)
#define REQUIRE_GE(expected, actual) REQUIRE_OP(expected, >=, actual)
#define REQUIRE_LE(expected, actual) REQUIRE_OP(expected, <=, actual)
#define REQUIRE_GT(expected, actual) REQUIRE_OP(expected, >,  actual)
#define REQUIRE_LT(expected, actual) REQUIRE_OP(expected, <,  actual)

#define REQUIRE_TRUE(condition)  REQUIRE_EQ(condition, true)
#define REQUIRE_FALSE(condition) REQUIRE_EQ(condition, false)

#define REQUIRE_FLOAT_EQ(expected, actual)                           \
	do {                                                             \
	    if(!fuzzyEqual(expected, actual))                            \
	      std::cerr << #expected << " expect " << expected << ", "   \
	                << #actual << " get " << actual << " "           \
	                << '@' << __FILE__ << "#L" << __LINE__ << '\n';  \
	} while(false)                                                   \

#endif  // PEA_UTIL_TEST_H_
