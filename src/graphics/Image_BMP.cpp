#include "graphics/Image_BMP.h"

#include <cassert>
#include <cstdlib>
#include <fstream>

#define __STDC_FORMAT_MACROS
#include <cinttypes>  // print format int64_t type

#include "util/Log.h"
#include "util/utility.h"


#define BIT_ARRAY(array, n) ((array[n>>3] & (1<<(n%8)))

static const char* TAG = "Image_BMP";
static constexpr uint16_t MARK = 'B'|('M'<<8);

using namespace pea;

//const uint8_t Image_BMP::MAGIC[2] = {'B', 'M'};

Image_BMP::Image_BMP():
		Image(),
		table(nullptr)
{

}

Image_BMP::Image_BMP(uint32_t width, uint32_t height, Color::Format format):
		Image(width, height, format),
		table(nullptr)
{
}

Image_BMP::Image_BMP(uint32_t width, uint32_t height, Color::Format format, uint8_t* data):
		Image(width, height, format, data),
		table(nullptr)
{
}

Image_BMP::~Image_BMP()
{
	SAFE_DELETE(table);
}

bool Image_BMP::probe(const uint8_t* data, size_t length)
{
	if(length <= sizeof(BITMAP_FILE_HEADER) + sizeof(BITMAP_INFO_HEADER))
		return false;
	
	return *reinterpret_cast<const uint16_t*>(data) == MARK;
}

std::shared_ptr<Image_BMP> Image_BMP::decodeByteArray(const uint8_t* data, size_t length)
{
	if(length <= sizeof(BITMAP_FILE_HEADER) + sizeof(BITMAP_INFO_HEADER))
		return nullptr;
	
	const BITMAP_FILE_HEADER& fileHeader = *reinterpret_cast<const BITMAP_FILE_HEADER*>(data);
	if(fileHeader.type != MARK)
		return nullptr;
	
	if(fileHeader.size > length)
	{
		slog.d(TAG, "not enough data (%zu < %" PRIu64 ")", fileHeader.size, length);
		return nullptr;
	}
	
	assert(fileHeader.reserved1 == 0);
	assert(fileHeader.reserved2 == 0);
	
	data += sizeof(BITMAP_FILE_HEADER);
	const BITMAP_INFO_HEADER& infoHeader = *reinterpret_cast<const BITMAP_INFO_HEADER*>(data);
	
	if(infoHeader.size + sizeof(BITMAP_FILE_HEADER) > fileHeader.offBits)
	{
		slog.d(TAG, "invalid BMP file info header");
		return nullptr;
	}

	Color::Format colorFormat = Color::Format::UNKNOWN;
	
	if(infoHeader.size == sizeof(BITMAP_INFO_HEADER))
		colorFormat = Color::RGB_888;
	else if(infoHeader.size == sizeof(BITMAP_V4_HEADER) ||
			infoHeader.size == sizeof(BITMAP_V5_HEADER))
	{
		data += sizeof(BITMAP_INFO_HEADER);
		const BITMAP_COLOR_MASK& colorMask = *reinterpret_cast<const BITMAP_COLOR_MASK*>(data);
	}
	else
	{
		slog.w(TAG, "Oops, wrong size or unsupported BMP version header");
		return nullptr;
	}
	
	uint32_t width = infoHeader.width;
	uint32_t height = std::abs(infoHeader.height);
	uint8_t* imageData = nullptr;
	if((width | height) <= 0) // (width <= 0 || height <=0)
	{
		slog.e(TAG, "bad image size: %dx%d", width, height);
		width = height = 0;
		goto bail;
	}

	if(infoHeader.planes != 1)
		slog.w(TAG, "invalid BMP info header, there should be only one plane.");

	// BitCount == 0 means the number of bits-per-pixel is specified or is
	// implied by the JPEG or PNG format.
	if(infoHeader.bitCount == 0 ||
			infoHeader.compression != COMPRESSION_RGB ||
			infoHeader.compression != COMPRESSION_RLE8 ||
			infoHeader.compression != COMPRESSION_RLE4 ||
			infoHeader.compression != COMPRESSION_BITFIELDS)
	{
		slog.i(TAG, "BMP compression %zu not supported", infoHeader.compression);
		goto bail;
	}
	
	// TODO:
bail:
	return std::make_shared<Image_BMP>(width, height, colorFormat, imageData);
}

std::shared_ptr<Image_BMP> Image_BMP::decodeFile(const std::string& path)
{
	std::shared_ptr<Image_BMP> image;
	std::ifstream file(path, std::ios::in | std::ios::binary);
	if(!file.is_open())
	{
		slog.w(TAG, "invalid path: %s", path.c_str());
		return image;
	}

	file.seekg(0, std::ios::end);
	size_t fileSize = file.tellg();  // std::ifstream::off_type
	file.seekg(0, std::ios::beg);

	BITMAP_FILE_HEADER fileHeader;
	BITMAP_INFO_HEADER infoHeader;
	BITMAP_COLOR_MASK  colorMask;
	
	uint32_t width = 0, height = 0;
	Color::Format colorFormat = Color::RGBA_8888;
	uint8_t* data = nullptr;
	
	if(fileSize <= sizeof(BITMAP_FILE_HEADER) + sizeof(BITMAP_INFO_HEADER))
	{
		slog.d(TAG, "not a BMP file!");
		goto bail;
	}

	file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
	if(fileHeader.type != MARK)
	{
		slog.d(TAG, "bad magic number");
		goto bail;
	}
	if(fileHeader.size > fileSize)
	{
		slog.d(TAG, "not enough data (%zu < %" PRIu64 ")", fileHeader.size, fileSize);
		goto bail;
	}
	assert(fileHeader.reserved1 == 0);
	assert(fileHeader.reserved2 == 0);
	
	file.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));
	if(infoHeader.size + sizeof(BITMAP_FILE_HEADER) > fileHeader.offBits)
	{
		slog.d(TAG, "invalid BMP file info header");
		goto bail;
	}

	if(infoHeader.size == sizeof(BITMAP_INFO_HEADER))
		colorFormat = Color::RGB_888;
	else if(infoHeader.size == sizeof(BITMAP_V4_HEADER) ||
			infoHeader.size == sizeof(BITMAP_V5_HEADER))
		file.read(reinterpret_cast<char*>(&colorMask), sizeof(BITMAP_COLOR_MASK));
	else
	{
		slog.w(TAG, "Oops, wrong size or unsupported BMP version header");
		goto bail;
	}

	// If height is positive, the bitmap is a bottom-up DIB, its origin being the
	// lower-left corner. If height is negative, the bitmap is a top-down DIB and
	// its origin being the upper-left corner. Top-down DIBs cannot be compressed.
	width = infoHeader.width;
	height = std::abs(infoHeader.height);
	if((width | height) <= 0) // (width <= 0 || height <=0)
	{
		slog.e(TAG, "bad image size: %dx%d", width, height);
		width = height = 0;
		goto bail;
	}

	if(infoHeader.planes != 1)
		slog.w(TAG, "invalid BMP info header, there should be only one plane.");

	// BitCount == 0 means the number of bits-per-pixel is specified or is
	// implied by the JPEG or PNG format.
	if(infoHeader.bitCount == 0)
	{
		slog.i(TAG, "#%d infoHeader.bitCount = 0 currently not supported", __LINE__);
		goto bail;
	}
	else if(infoHeader.compression != COMPRESSION_RGB &&
			infoHeader.compression != COMPRESSION_RLE8 &&
			infoHeader.compression != COMPRESSION_RLE4 &&
			infoHeader.compression != COMPRESSION_BITFIELDS)
	{
		slog.i(TAG, "#%d BMP compression %zu not supported", __LINE__, infoHeader.compression);
		goto bail;
	}

	// 1-, 4-, and 8-bit BMP files are expected to always contain a color palette.
	// 16-, 24-, and 32-bit BMP files never contain color palettes. Sixteen- and
	// 32-bit BMP files contain bitfields mask values in place of the color palette.
	if(infoHeader.bitCount == 1 ||
			infoHeader.bitCount == 4 ||
			infoHeader.bitCount == 8)
	{
		size_t colorCount = 1 << infoHeader.bitCount;
		Color* palette = new (std::nothrow) Color[colorCount];
		if(!palette)
		{
			goto bail;
		}

		file.read(reinterpret_cast<char*>(palette), colorCount * sizeof(Color));

		delete[] palette;
	}

	switch(infoHeader.bitCount)
	{
	case 32:
		if(infoHeader.compression == COMPRESSION_BITFIELDS)
		{
			if(colorMask.redMask == 0x000000FF &&
					colorMask.greenMask == 0x0000FF00 &&
					colorMask.blueMask == 0x00FF0000)
				colorFormat = colorMask.alphaMask? Color::RGBA_8888 : Color::RGB_888;
			else if(colorMask.redMask == 0x00FF0000 &&
					colorMask.greenMask == 0x0000FF00 &&
					colorMask.blueMask == 0x000000FF)
				colorFormat = colorMask.alphaMask? Color::BGRA_8888 : Color::BGR_888;
			else
				slog.d(TAG, "unknown bit mask 0x%0X 0x%0X 0x%0X 0x%0X", colorMask.redMask,
						colorMask.greenMask, colorMask.blueMask, colorMask.alphaMask);
		}
		else
			colorFormat = Color::RGBA_8888;
		break;
	case 24:
		colorFormat = Color::RGB_888;
		break;
	case 16:
		break;
	case 8:
		colorFormat = Color::G_8;
		break;
	case 4:
	case 1:
	default:
		slog.d(TAG, "invalid BitCount value: %d" PRIu16, infoHeader.bitCount);
//		goto bail;
		assert(false);  // unhandled case
		break;
	}

//	_depth = infoHeader.BitCount;
	file.seekg(sizeof(fileHeader) + fileHeader.size);
//	fseek(file, sizeof(fileHeader) + fileHeader.Size, SEEK_SET);

//	size_t size = width * height;
	data = new (std::nothrow) uint8_t[width * height * Color::size(colorFormat)];
	if(data)
	{
	}
	
/*
	int pitch = (width + 3) & ~3;
	uint8_t *line = new uint8_t[pitch];

	SAFE_DELETE_ARRAY(data);
	SAFE_DELETE_ARRAY(line);
*/
/*
	if(infoHeader.BitCount == 1)
	{
		for(int s = 0; s < _height; ++s)
		{
			fread(line, pitch, 1, file);
			for(size_t t=0; t < _width; ++t)
				data[s*_width+t] = table[line[t/8]&(1<<(t%8))];
		}
	}
	else if(infoHeader.BitCount == 4)
	{
		for(int s=0; s < _height; ++s)
		{
			fread(line, pitch, 1, file);
			for(size_t t = 0; t < _width; ++t)
				data[s*_width+t] = table[line[t/2]&(15<<(t%2))];
		}
	}
	else if(infoHeader.BitCount == 8)
	{
		for(int s=0; s < _height; ++s)
		{
			fread(line, pitch, 1, file);
			for(size_t t=0; t < _width; ++t)
				data[s*_width+t] = table[line[t]];
		}
	}
	else if(infoHeader.BitCount == 16)
	{
		for(int s=0;s < _height; ++s)
		{
			fread(line, pitch, 1, file);
			for(size_t t=0; t<(_width<<1); t+=2)
				data[s*_width+t] = table[(int)line[t]+(line[t+1]<<8)];
		}
	}
	else if(infoHeader.BitCount == 24)
	{
		for(int s=0; s < _height; ++s)
		{
			fread(line, pitch, 1, file);
			const size_t bytes = _width*3;
			for(size_t t=0; t<bytes; t+=3)
				data[s*_width+t] = Color(line[t],line[t+1],line[t+2],255);
		}
	}
	else if(infoHeader.BitCount == 32)
	{
		Color *p=data;
		const long block = sizeof(Color)*_width;
		for(int s=0; s < _height; ++s)
		{
			fread(p, block, 1, file);
//			fseek(file, ,SEEK_SET);
			p += block;
		}
	}
	else
	{
		slog.e(TAG, "invalid BitCount value: %d\n", infoHeader.BitCount);
		file.close();
		return false;
	}
*/


//	int index=0;
//	switch(infoHeader.Compression)
//	{
//	case 0: // COMPRESSION_RGB, An uncompressed format
//		break;
//	case 1: // COMPRESSION_RLE8, A run-length encoded (RLE) format for bitmaps with 8 bpp
//	case 2: // An RLE format for bitmaps with 4 bpp
//	case 3: // COMPRESSION_BITFIELDS
//	case 4: // COMPRESSION_JPEG
//	case 5: // COMPRESSION_PNG
//		assert(false);
//		break;
//	}

//	free(line);
bail:
	file.close();
	return std::make_shared<Image_BMP>(width, height, colorFormat, data);
}
/*
bool Image_BMP::load(const void* data, DataType type, long width, long height)
{


	this->data = new BYTE[data_size]; // XXX
	Color *p_data_to = this->data;
	Color *p_data_from = (Color*)data;

	memset(p_data_to, 0, data_size);
	for(size_t i=0; i<height; ++i)
	{
		memcpy(p_data_to, p_data_from, width*sizeof(Color));
		p_data_from += width;
		p_data_to += pitch;
	}

	return true;
}
*/
bool Image_BMP::save(const std::string& path) const
{
	std::ofstream file(path, std::ios::out | std::ios::binary);
	if(!file.is_open())
	{
		slog.w(TAG, "can't open file %s for writing", path.c_str());
		return false;
	}

	// TODO: BMP format removes alpha channel?
	uint32_t pitch = width * sizeof(Color);  // sizeof(Color) are 4 bytes aligned already.
	uint32_t dataSize = pitch * height;

	const uint32_t headerSize = sizeof(BITMAP_FILE_HEADER) + sizeof(BITMAP_INFO_HEADER);
	BITMAP_FILE_HEADER fileHeader =
	{
		MARK,                     // type
		headerSize + dataSize,    // offBits
		0,                        // reserved1
		0,                        // reserved2
		headerSize                // size
	};

	BITMAP_INFO_HEADER infoHeader =
	{
		sizeof(BITMAP_INFO_HEADER), // size
		static_cast<int32_t>(width),// width
		-static_cast<int32_t>(height),// height, negative makes top-to-down
		1,                          // planes
		32,                         // bitCount
		COMPRESSION_RGB,            // compression = 0, an uncompressed format.
		dataSize,                   // sizeImage in bytes (including padding)
		0,                          // xPixelsPerMeter
		0,                          // yPixelsPerMeter
		0,                          // colorUsed
		0                           // colorImportant
	};

	// write 32 bit RGBA color mode, no palette
	file.write(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
	file.write(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));
	file.write(reinterpret_cast<char*>(this->data), dataSize);
	file.close();

	return true;
}
