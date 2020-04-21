#include "test/catch.hpp"

#include "util/Base64.h"

static const char* tag = "[utility]";


using namespace pea;

TEST_CASE("Base64", tag)
{
	using StringPair = std::pair<std::string, std::string>;
	const StringPair codes[] =
	{
		StringPair("P", "UA=="),
		StringPair("Pe", "UGU="),
		StringPair("Pea", "UGVh"),
		StringPair("https://github.com/KAlO2/Pea.git", "aHR0cHM6Ly9naXRodWIuY29tL0tBbE8yL1BlYS5naXQ=" )

	};

	size_t N = sizeof(codes) / sizeof(codes[0]);  // LENGTH_OF_ARRAY(code);
	for(size_t i = 0; i < N; ++i)
	{
		const std::string& decrypt = codes[i].first;
		const std::string& encrypt = codes[i].second;
		REQUIRE(Base64::encode(decrypt) == encrypt);
		REQUIRE(Base64::decode(encrypt) == decrypt);
	}
}
