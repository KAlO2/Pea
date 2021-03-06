#include "test/catch.hpp"

#include <sstream>

#include "pea/config.h"
#include "graphics/ImageFactory.h"
#include "graphics/Image_BMP.h"
#include "graphics/Image_JPG.h"
#include "graphics/Image_PNG.h"
#include "graphics/Image_TGA.h"
#include "io/FileSystem.h"
#include "util/Log.h"
#include "util/utility.h"


using namespace pea;

static const char* tag = "[image]";
static const char* TAG = "image";

TEST_CASE("Color", tag)
{
	using Format = Color::Format;
	REQUIRE(Color::sizeofChannel(Format::C1_U8) == 1U);
	REQUIRE(Color::sizeofChannel(Format::C2_U8) == 2U);
	REQUIRE(Color::sizeofChannel(Format::C3_U8) == 3U);
	REQUIRE(Color::sizeofChannel(Format::C4_U8) == 4U);
	
	REQUIRE(Color::sizeofChannel(Format::RGBA5551_U16) == 4U);
	REQUIRE(Color::sizeofChannel(Format::RGBA4444_U16) == 4U);
	REQUIRE(Color::sizeofChannel(Format::RGB565_U16) == 3U);
	REQUIRE(Color::sizeofChannel(Format::RGBA1010102_U32) == 4U);
	
	REQUIRE(Color::sizeofChannel(Format::BGR888_U24) == 3U);
	REQUIRE(Color::sizeofChannel(Format::BGRA8888_U32) == 4U);
	
	REQUIRE(!Color::isFloatType(Format::UNKNOWN));
	REQUIRE(!Color::isIntegerType(Format::UNKNOWN));
	REQUIRE(Color::isFloatType(Format::C1_F16));
	REQUIRE(!Color::isIntegerType(Format::C1_F32));
	REQUIRE(!Color::isFloatType(Format::C4_U8));
	REQUIRE(!Color::isFloatType(Format::BGR888_U24));
	REQUIRE(!Color::isFloatType(Format::BGRA8888_U32));
	REQUIRE(!Color::isFloatType(Format::C4_U32));
	
	REQUIRE(Color::size(Format::C1_U8) == 1);
	REQUIRE(Color::size(Format::C4_U8) == 4);
	REQUIRE(Color::size(Format::C4_F32) == 4 * sizeof(float));
}

TEST_CASE("Image even", tag)
{
	constexpr uint32_t width = 2, height = 2;
	uint32_t pixels[] =
	{
		0x00112233, 0x44556677,
		0x8899AABB, 0xCCDDEEFF,
	};
	
	uint32_t pixels_h[] =
	{
		0x44556677, 0x00112233,
		0xCCDDEEFF, 0x8899AABB,
	};
	
	uint32_t pixels_v[] =
	{
		0x8899AABB, 0xCCDDEEFF,
		0x00112233, 0x44556677,
	};
	
	Image_PNG image(width, height, Color::C4_U8);
	
	for(uint32_t y = 0; y < height; ++y)
		for(uint32_t x = 0; x < width; ++x)
			image.setPixel(x, y, pixels[y * width + x]);
	
	image.flipHorizontal();
	for(uint32_t y = 0; y < height; ++y)
		for(uint32_t x = 0; x < width; ++x)
			REQUIRE(image.getPixel(x, y) == pixels_h[y * width + x]);
	
	image.flipHorizontal();
	for(uint32_t y = 0; y < height; ++y)
		for(uint32_t x = 0; x < width; ++x)
			REQUIRE(image.getPixel(x, y) == pixels[y * width + x]);
	
	image.flipVertical();
	for(uint32_t y = 0; y < height; ++y)
		for(uint32_t x = 0; x < width; ++x)
			REQUIRE(image.getPixel(x, y) == pixels_v[y * width + x]);
}

TEST_CASE("Image odd", tag)
{
	constexpr uint32_t width = 3, height = 3;
	uint8_t pixels[] =
	{
		0, 1, 2,
		3, 4, 5,
		6, 7, 8,
	};
	
	uint8_t pixels_h[] =
	{
		2, 1, 0,
		5, 4, 3,
		8, 7, 6,
	};
	
	uint8_t pixels_v[] =
	{
		6, 7, 8,
		3, 4, 5,
		0, 1, 2,
	};
	
	Image_PNG image(width, height, Color::C1_U8);
	
	for(uint32_t y = 0; y < height; ++y)
		for(uint32_t x = 0; x < width; ++x)
			image.setPixel(x, y, Color::from_G8(pixels[y * width + x]));
	
	image.flipHorizontal();
	for(uint32_t y = 0; y < height; ++y)
		for(uint32_t x = 0; x < width; ++x)
			REQUIRE(image.getPixel(x, y) == Color::from_G8(pixels_h[y * width + x]));
	
	image.flipHorizontal();
	for(uint32_t y = 0; y < height; ++y)
		for(uint32_t x = 0; x < width; ++x)
			REQUIRE(image.getPixel(x, y) == Color::from_G8(pixels[y * width + x]));
	
	image.flipVertical();
	for(uint32_t y = 0; y < height; ++y)
		for(uint32_t x = 0; x < width; ++x)
			REQUIRE(image.getPixel(x, y) == Color::from_G8(pixels_v[y * width + x]));
}

TEST_CASE("Image_BMP", tag)
{
	Image_BMP image(1280, 720, Color::C4_U8);
	image.fillCheckerboard(240);

	std::string filename = "checkerboard.rgba.bmp";
	bool flag = image.save(filename);
	REQUIRE(flag);
	slog.i(TAG, "save BMP image %s", filename.c_str());
}

TEST_CASE("Image_PNG", tag)
{
	Image_PNG image(1024, 1024, Color::C4_U8);
	image.fillCheckerboard(128);
	
	std::string filename = "checkerboard.rgba.png";
	bool flag = image.save(filename);
	REQUIRE(flag);
	slog.i(TAG, "save PNG image %s", filename.c_str());
}

#if JPEG_FOUND
TEST_CASE("Image_JPG", tag)
{
	// A Complete Story of Lenna @see http://www.ee.cityu.edu.hk/~lmpo/lenna/Lenna97.html
	std::string path = FileSystem::getRelativePath("res/image/lena_std.jpg");
	std::shared_ptr<Image> image = ImageFactory::decodeFile(path);
	REQUIRE(image.get());
	REQUIRE(image->getWidth() == 256);
	REQUIRE(image->getHeight() == 256);
	REQUIRE(image->getImageFormat() == Image::Format::JPG);
	
	std::string dir = FileSystem::dirname(path);
	std::string filename = FileSystem::basename(path);
	size_t pos = filename.rfind('.', std::string::npos);
	std::string name = filename.substr(0, pos);

	int32_t quality = 95;
	std::ostringstream str;
	str << '.' << FileSystem::SEPERATOR << name
			<< '.' << image->getWidth() << 'x' << image->getHeight()
			<< "_quality_" << quality << filename.substr(pos);

	std::string output = str.str();
	slog.i(TAG, "save JPG image to path \"%s\"", output.c_str());

	Image_JPG& lena = *dynamic_cast<Image_JPG*>(image.get());
	lena.setQuality(quality);
	bool flag = lena.save(output);
	REQUIRE(flag);
	
	lena.flipVertical();
	flag = lena.save(name + ".flipVertical.jpg");
	REQUIRE(flag);
	
	lena.flipVertical();  // restore
	lena.flipHorizontal();
	flag = lena.save(name + ".flipHorizontal.jpg");
	REQUIRE(flag);
}
#endif  // JPEG_FOUND

void openImage(const std::string path)
{
	std::ostringstream stream;
	
#ifdef __linux__
	// use eog command, which is a GNOME image viewer.
	stream << "eog" << ' ';
#elif defined(_WIN32)
//	snprintf(cmd, sizeof(cmd), "\"%s\"", path.c_str());
#elif defined(__APPLE__)
	stream << "open" << ' ';
#endif

	// In case of space in command, filename should always enclose with ""
	stream << '\"' << path << '\"';
	std::string command = stream.str();
	int ret = system(command.c_str());
	if(ret != 0)
		slog.i(TAG, "system command error (%d)", ret);
}

TEST_CASE("Image_TGA", tag)
{
	const std::string filenames[] =
	{
		"opengl_logo_uncompressed.tga",
		"opengl_logo_compressed.tga",
	};
	constexpr size_t size = sizeofArray(filenames);
	constexpr uint32_t width = 512, height = 256;
	
	std::shared_ptr<Image> images[size];
	for(size_t i = 0; i < size; ++i)
	{
		const std::string& filename = filenames[i];
		std::string path = FileSystem::getRelativePath("res/image/" + filename);
		slog.i(TAG, "path=%s", path.c_str());
		REQUIRE(ImageFactory::probe(path) == Image::Format::TGA);
		std::shared_ptr<Image> image = ImageFactory::decodeFile(path);
		
		REQUIRE(image.get());
		REQUIRE(image->getWidth() == width);
		REQUIRE(image->getHeight() == height);
		REQUIRE(image->getImageFormat() == Image::Format::TGA);
		
		REQUIRE(dynamic_cast<Image_TGA*>(image.get()) != nullptr);
		images[i] = image;
	}
/*
	for(uint32_t y = 0; y < height; ++y)
	for(uint32_t x = 0; x < width;  ++x)
	{
		uint32_t color1 = images[0]->getPixel(x, y);
		uint32_t color2 = images[1]->getPixel(x, y);
		REQUIRE(color1 == color2);
	}
*/
}

TEST_CASE("ColorF", tag)
{
	vec4f white(1.0, 1.0, 1.0, 1.0);
	REQUIRE(ColorF::getColor(white) == 0xFFFFFFFF);
	
	vec4f black(0.0, 0.0, 0.0, 1.0);
	REQUIRE(ColorF::getColor(black) == 0xFF000000);
	
	vec4f red(1.0, 0.0, 0.0, 1.0);
	REQUIRE(ColorF::getColor(red) == 0xFF0000FF);
	
	vec4f gray(0.5, 0.5, 0.5, 1.0);
	REQUIRE(ColorF::getColor(gray) == 0xFF7F7F7F);
}
