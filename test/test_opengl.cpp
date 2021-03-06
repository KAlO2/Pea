#include "test/catch.hpp"

#include "io/Model_glTF2.h"
#include "opengl/GL.h"
#include "opengl/Texture.h"
#include "util/compiler.h"

using namespace pea;

static const char* tag = "[opengl]";

TEST_CASE("Texture::Parameter", tag)
{
//	using pea::glTF2::MinFilter;
	REQUIRE(underlying_cast(MinFilter::NEAREST) == GL_NEAREST);
	REQUIRE(underlying_cast(MinFilter::LINEAR) == GL_LINEAR);
	REQUIRE(underlying_cast(MinFilter::LINEAR) == GL_LINEAR);
	REQUIRE(underlying_cast(MinFilter::NEAREST_MIPMAP_NEAREST) == GL_NEAREST_MIPMAP_NEAREST);
	REQUIRE(underlying_cast(MinFilter::LINEAR_MIPMAP_NEAREST)  == GL_LINEAR_MIPMAP_NEAREST);
	REQUIRE(underlying_cast(MinFilter::NEAREST_MIPMAP_LINEAR)  == GL_NEAREST_MIPMAP_LINEAR);
	REQUIRE(underlying_cast(MinFilter::LINEAR_MIPMAP_LINEAR)   == GL_LINEAR_MIPMAP_LINEAR);

//	using pea::glTF2::MagFilter;
	REQUIRE(underlying_cast(MagFilter::NEAREST) == GL_NEAREST);
	REQUIRE(underlying_cast(MagFilter::LINEAR) == GL_LINEAR);

//	using pea::glTF2::WrapMode;
	REQUIRE(underlying_cast(WrapMode::CLAMP)  == GL_CLAMP);
	REQUIRE(underlying_cast(WrapMode::REPEAT) == GL_REPEAT);
	REQUIRE(underlying_cast(WrapMode::CLAMP_TO_BORDER) == GL_CLAMP_TO_BORDER);
	REQUIRE(underlying_cast(WrapMode::CLAMP_TO_EDGE)    == GL_CLAMP_TO_EDGE);
	REQUIRE(underlying_cast(WrapMode::MIRRORED_REPEAT)  == GL_MIRRORED_REPEAT);
}

TEST_CASE("ComponentType", tag)
{
	using glTF2::ComponentType;
	REQUIRE(underlying_cast(ComponentType::BYTE) == GL_BYTE);
	REQUIRE(underlying_cast(ComponentType::UNSIGNED_BYTE) == GL_UNSIGNED_BYTE);
	REQUIRE(underlying_cast(ComponentType::SHORT) == GL_SHORT);
	REQUIRE(underlying_cast(ComponentType::UNSIGNED_SHORT) == GL_UNSIGNED_SHORT);
	REQUIRE(underlying_cast(ComponentType::UNSIGNED_INT) == GL_UNSIGNED_INT);
	REQUIRE(underlying_cast(ComponentType::FLOAT) == GL_FLOAT);
}
