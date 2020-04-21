#include "test/catch.hpp"

#include "geometry/Cube.h"
#include "io/Model.h"
#include "io/Model_MTL.h"
#include "io/Model_OBJ.h"
#include "io/File.h"
#include "util/platform.h"

static const char* TAG = "Model";


using namespace pea;

TEST_CASE("dirname basename", tag)
{
	const char* array[][3] =
	{	// path      dirname basename
		{"/usr/lib", "/usr", "lib"},
		{"/usr/",    "/",    "usr"},
		{"usr",      ".",    "usr"},
		{"/",        "/",    "/"  },
		{".",        ".",    "."  },
		{"..",       ".",    ".." },
		{"",         "",     ""   },
	};
	
	for(size_t i = 0; i < sizeofArray(array); ++i)
	{
		std::string path = array[i][0];
		std::string dirnameExpect = array[i][1];
		std::string basenameExpect = array[i][2];
		REQUIRE(dirnameExpect == FileSystem::dirname(path));
		REQUIRE(basenameExpect == FileSystem::basename(path));
	}
}

TEST_CASE(".obj triple index parser", tag)
{
	REQUIRE(vec3u(1u, 2u, 3u) == Type::parseUint3("1/2/3"));
	REQUIRE(vec3u(1u, 0u, 3u) == Type::parseUint3("1//3"));
	REQUIRE(vec3u(1u, 2u, 0u) == Type::parseUint3("1/2/"));
}

