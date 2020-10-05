#include "test/catch.hpp"

#include "opengl/IndexBuffer.h"

using namespace pea;

static const char* tag = "[opengl]";

TEST_CASE("IndexBuffer", tag)
{
	IndexBuffer buffer0(IndexBuffer::Type::UINT8);
	REQUIRE(buffer0.data() == nullptr);
	REQUIRE(buffer0.length() == 0);
/*
	IndexBuffer buffer1(255, 256);
	REQUIRE(buffer1.getType() == IndexBuffer::Type::UINT8);
	REQUIRE(buffer1.length() == 256);
*/
	const uint32_t LENGTH = 6;
	std::unique_ptr<uint16_t[]> array = std::make_unique<uint16_t[]>(LENGTH);
	for(uint32_t i = 0; i < LENGTH; ++i)
		array.get()[i] = i;
	
	IndexBuffer buffer2(std::move(array), LENGTH);
	REQUIRE(array.get() == nullptr);
	REQUIRE(buffer2.getType() == IndexBuffer::Type::UINT16);
	REQUIRE(buffer2.length() == LENGTH);
	
	const IndexBuffer::Type shrinkType = IndexBuffer::Type::UINT8;
	REQUIRE(buffer2.getCompactType() == shrinkType);
	buffer2.changeType(shrinkType);
	REQUIRE(buffer2.getType() == shrinkType);
	REQUIRE(buffer2.length() == LENGTH);
	
}
