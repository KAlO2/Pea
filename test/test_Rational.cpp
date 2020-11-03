#include "test/catch.hpp"

#include "util/Rational.h"
#include <iostream>
using namespace pea;

static const char* tag = "[math]";

TEST_CASE("Rational", tag)
{
	using Ratio = Rational<int32_t>;
	REQUIRE(Ratio(1, 2) == Ratio(1, 2));
	REQUIRE(Ratio(1, 2) == Ratio(2, 4));
	REQUIRE(Ratio(0, 0) != Ratio(0, 0));
	REQUIRE(Ratio(1, 0) == Ratio(2, 0));
	
	REQUIRE(Ratio(0, 0).isNaN());
	REQUIRE(Ratio(0, 2).isFinite());
	REQUIRE(!Ratio(2, 0).isFinite());
	REQUIRE(Ratio(0, 3).isZero());
	
	REQUIRE(Ratio(1, 2) == Ratio(1, 2));
/*
	2/3 + 1/6 = 5/6
	2/3 - 1/6 = 1/2
	2/3 * 1/6 = 1/9
	(2/3) / (1/6) = 4/1
*/
	REQUIRE(Ratio(2, 3) + Ratio(1, 6) == Ratio(5, 6));
	REQUIRE(Ratio(2, 3) - Ratio(1, 6) == Ratio(1, 2));
	REQUIRE(Ratio(2, 3) * Ratio(1, 6) == Ratio(1, 9));
	REQUIRE(Ratio(2, 3) / Ratio(1, 6) == Ratio(4, 1));
	
	// https://en.cppreference.com/w/cpp/numeric/ratio/ratio_less
	REQUIRE(Ratio(23, 37) < Ratio(57, 90));
	// https://en.cppreference.com/w/cpp/numeric/ratio/ratio_equal
	REQUIRE(Ratio(2, 3) == Ratio(4, 6));
	// https://en.cppreference.com/w/cpp/numeric/ratio/ratio_not_equal
	REQUIRE(Ratio(2, 3) != Ratio(1, 3));
	
	// https://en.cppreference.com/w/cpp/numeric/ratio/ratio_greater_equal
	REQUIRE(Ratio(2, 1) >= Ratio(1, 2));
	REQUIRE(Ratio(1, 2) >= Ratio(1, 2));
	REQUIRE(Ratio(999'999, 1'000'000) >= Ratio(999'998, 999'999));
	REQUIRE(Ratio(999'999, 1'000'000) >= Ratio(999'999, 1'000'000));
	
	// https://en.cppreference.com/w/cpp/numeric/ratio/ratio_greater
	REQUIRE(Ratio(11, 12) > Ratio(10, 11));
	REQUIRE(Ratio(12, 13) > Ratio(11, 12));
	
	// https://en.cppreference.com/w/cpp/numeric/ratio/ratio_less_equal
	REQUIRE(Ratio(10, 11) <= Ratio(11, 12));
	REQUIRE(Ratio(11, 12) <= Ratio(12, 13));
	
	REQUIRE(Ratio(1, 2).reciprocal() == Ratio(2, 1));
	REQUIRE(Ratio::POSITIVE_INFINITY.reciprocal().isZero());
	REQUIRE(Ratio::NEGATIVE_INFINITY.reciprocal().isZero());
	REQUIRE(Ratio::NaN.reciprocal().isNaN());
}

TEST_CASE("Rational to infinity and nan", tag)
{
	using Ratio = Rational<int32_t>;
	REQUIRE((Ratio(1, -2) < Ratio::NaN) == false);
	REQUIRE((Ratio(1, -2) > Ratio::NaN) == false);
	
	REQUIRE(Ratio(1, 2) < Ratio::POSITIVE_INFINITY);
	REQUIRE(Ratio(1, 2) > Ratio::NEGATIVE_INFINITY);
	REQUIRE(Ratio(1, -2) > Ratio::NEGATIVE_INFINITY);
	REQUIRE(Ratio(0, -1) > Ratio::NEGATIVE_INFINITY);
	REQUIRE(Ratio(0, 1) == Ratio::ZERO);
	
	REQUIRE(Ratio::NEGATIVE_INFINITY < Ratio::POSITIVE_INFINITY);  // -inf < +inf
	REQUIRE(Ratio(1, -2) + Ratio::POSITIVE_INFINITY == Ratio::POSITIVE_INFINITY);
	
	// inf + inf = inf, inf - inf = nan, inf * inf = inf, inf / inf = nan
	REQUIRE(Ratio::POSITIVE_INFINITY + Ratio::POSITIVE_INFINITY == Ratio::POSITIVE_INFINITY);
	REQUIRE((Ratio::POSITIVE_INFINITY - Ratio::POSITIVE_INFINITY).isNaN());
	REQUIRE(Ratio::POSITIVE_INFINITY * Ratio::POSITIVE_INFINITY == Ratio::POSITIVE_INFINITY);
	REQUIRE((Ratio::POSITIVE_INFINITY * Ratio::ZERO).isNaN());
	
	REQUIRE((Ratio::POSITIVE_INFINITY / Ratio::POSITIVE_INFINITY).isNaN());
	REQUIRE(Ratio(0, 2) / Ratio::POSITIVE_INFINITY == Ratio::ZERO);  // 0 / +inf = 0
	REQUIRE(Ratio(0, 2) / Ratio::NEGATIVE_INFINITY == Ratio::ZERO);  // 0 / -inf = 0
	REQUIRE((Ratio(0, 2) * Ratio::POSITIVE_INFINITY).isNaN());  // 0 * inf = NaN
}
