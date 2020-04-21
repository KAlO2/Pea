#include "graphics/ImageFactory.h"

#include "pea/config.h"
#include "graphics/Image_BMP.h"
#ifdef JPEG_FOUND
#include "graphics/Image_JPG.h"
#endif
#include "graphics/Image_PNG.h"
#include "graphics/Image_TGA.h"

#include "util/Log.h"

#include <string.h>
#include <vector>

static const char* TAG = "ImageFactory";

using namespace pea;

Image::Format ImageFactory::probe(const uint8_t* data, size_t length)
{
	if(data == nullptr || length < 8)
		return Image::Format::UNKNOWN;

	if(Image_PNG::probe(data, length))
		return Image::Format::PNG;
#ifdef JPEG_FOUND
	if(Image_JPG::probe(data, length))
		return Image::Format::JPG;
#endif
	if(Image_BMP::probe(data, length))
		return Image::Format::BMP;
	if(Image_TGA::probe(data, length))
		return Image::Format::TGA;

	return Image::Format::UNKNOWN;
}

Image::Format ImageFactory::probe(const std::string& path)
{
	Image::Format format = Image::Format::UNKNOWN;
	const char* str = path.c_str();
	const char* pos = strrchr(str, '.');
	if(pos)
	{
		pos += 1;  // skip current dot to suffix
		if(!strcmp(pos, "png"))
			format = Image::Format::PNG;
		else if(!strcmp(pos, "tga"))
			format = Image::Format::TGA;
#ifdef JPEG_FOUND
		else if(!strcmp(pos, "jpg") || !strcmp(pos, "jpeg"))
			format = Image::Format::JPG;
#endif
		else if(!strcmp(pos, "bmp"))
			format = Image::Format::BMP;
		else if(!strcmp(pos, "tiff") || !strcmp(pos, "tif"))
			format = Image::Format::TIFF;
	}
	else
		slog.v(TAG, "path [%s] has no suffix.", str);
	
#if 0 // debug only

	FILE *file = fopen(str, "rb");
	if(!file)
	{
		slog.w(TAG, "can't open file %s for reading", str);
		return Image::Format::UNKNOWN;
	}
	fseek(file, 0L, SEEK_END);
	size_t length = ftell(file);  // long/ssize_t => size_t
	rewind(file);
	std::vector<uint8_t> image(length);  // TODO big size image will cause stack overflow for mobile devices?
	(void)fread(image.data(), length, 1, file);
	assert(format == probe(image.data(), length));  // file extension and magic number doesn't match.

#endif

	return format;
/*
	size_t pos = path.rfind('.', std::string::npos);
	if(pos == std::string::npos)
		return Format::UNKNOWN;

	const std::string suffix = path.substr(pos + 1);
//	if(suffix.com)  // case compare missing

	unsigned char magic[8] = {0};
	std::ifstream file(path, std::ios::in | std::ios::binary);
	if(!file.is_open())
	{
		slog.w(TAG, "invalid path: %s", path.c_str());
		return Format::UNKNOWN;
	}

*/
}

std::shared_ptr<Image> ImageFactory::decodeFile(const std::string& path)
{
	std::shared_ptr<Image> image;
	
	Image::Format imageFormat = probe(path);
	switch(imageFormat)
	{
	case Image::Format::PNG:
		image = Image_PNG::decodeFile(path);
		break;
#ifdef JPEG_FOUND
	case Image::Format::JPG:
		image = Image_JPG::decodeFile(path);
		break;
#endif
	case Image::Format::BMP:
		image = Image_BMP::decodeFile(path);
		break;
	case Image::Format::TGA:
		image = Image_TGA::decodeFile(path);
		break;
	default:
		assert(false);
		break;
	}
	
	return image;
}

std::shared_ptr<Image> ImageFactory::decodeByteArray(const uint8_t* data, size_t length)
{
	std::shared_ptr<Image> image;
	
	Image::Format imageFormat = probe(data, length);
	switch(imageFormat)
	{
	case Image::Format::PNG:
		image = Image_PNG::decodeByteArray(data, length);
		break;
#ifdef JPEG_FOUND
	case Image::Format::JPG:
		image = Image_JPG::decodeByteArray(data, length);
		break;
#endif
	case Image::Format::BMP:
		image = Image_BMP::decodeByteArray(data, length);
		break;
	case Image::Format::TGA:
		image = Image_TGA::decodeByteArray(data, length);
		break;
	default:
		assert(false);
		break;
	}
	
	return image;
}

Image::Format ImageFactory::probeFileName(const std::string& path)
{
	size_t delim = path.rfind('.');
	if(delim <= 0 || delim == std::string::npos)
		return Image::Format::UNKNOWN;
	
	std::string format(path, delim + 1);
	if(format == "png")
		return Image::Format::PNG;
	else if(format == "tga")
		return Image::Format::TGA;
#ifdef JPEG_FOUND
	else if(format == "jpg" || format == "jpeg")
		return Image::Format::JPG;
#endif
	else if(format == "bmp")
		return Image::Format::BMP;
//	else if(format == "xxx")
//		image = new (std::nothrow) Image_XXX;
	else
		slog.w(TAG, "unimplemented image format [%s].\n", format.c_str());

	return Image::Format::UNKNOWN;
}

bool ImageFactory::save(const std::string& path, const Image* image)
{
	assert(image && image->isValid());

	Image::Format dst_format = probe(path);
	if(dst_format == Image::Format::UNKNOWN)
	{
		size_t delim = path.rfind('.');
		if(delim != std::string::npos)
		{
			std::string format(path, delim + 1);
			slog.w(TAG, "unimplemented image format [%s].", format.c_str());
		}
		else if(delim <= 0)
			slog.w(TAG, "empty path", path.c_str());
	}

	Image::Format src_format = image->getImageFormat();
	if(src_format != dst_format)
	{
		slog.i(TAG, "convert format from %d to %d",
				static_cast<int>(src_format), static_cast<int>(dst_format));
		// TODO
		assert(false);
	}

	image->save(path);
	return true;
}
