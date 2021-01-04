#include "graphics/Image_TGA.h"

#include <cstring>
#include <ctime>
#include <fstream>
#include <vector>

#include "util/Log.h"
#include "util/platform.h"


static const char* TAG = "Image_TGA";

using namespace pea;

// unlike other image formats, TGA's magic number is at the end of file.
static const uint8_t MAGIC[18] = "TRUEVISION-XFILE.";


Image_TGA::Image_TGA():
		Image_TGA(0, 0, Color::Format::C4_U8, nullptr, true)
{
}

Image_TGA::Image_TGA(uint32_t width, uint32_t height, Color::Format format, uint8_t* data, bool move):
		Image(width, height, format, data, move),
		xOrigin(0),
		yOrigin(0),
		descriptor(static_cast<Descriptor>(0))
{
	static_assert(sizeof(Header) == 18, "TGA header should be one byte aligned");
	static_assert(sizeof(Extension) == 495, "TGA extension should be one byte aligned");
}

Image_TGA::~Image_TGA()
{

}

/*
	size_t length = rle(nullptr, src, n);
	uint8_t* dst = new uint8_t[length];
	if(dst)
		rle(dst, src, n);
*/
static size_t rle(uint8_t* dst, const uint8_t* src, size_t n)
{
	assert(n%2 == 0);  // one char followed by it's length
	size_t length = 0;
	for(size_t i = 1; i < n; i += 2)
		length += src[i];

	if(dst)
	{
		for(size_t i = 0; i < n; )
		{
			char ch = src[i++];
			uint8_t repeat = src[i++];
			while(repeat > 0)
				*dst++ = ch;
		}
	}

	return length;
}

bool Image_TGA::probe(const uint8_t* data, size_t length)
{
	if(length < sizeof(MAGIC))
		return false;
	
	data += length - sizeof(MAGIC);
	return std::memcmp(data, MAGIC, sizeof(MAGIC) - 1) == 0;
}

std::shared_ptr<Image_TGA> Image_TGA::decodeByteArray(const uint8_t* data, size_t length)
{
	uint32_t width = 0, height = 0;
	Color::Format colorFormat = Color::C4_U8;
	uint8_t* imageData = nullptr;
	
	if(length < sizeof(Header))
		return nullptr;

	const Header* header = reinterpret_cast<const Header*>(data);
	width = header->width;
	height = header->height;
//	uint16_t xOrigin = header->xOrigin;
//	uint16_t yOrigin = header->yOrigin;

	// TODO big endian with BYTE2SWAP
	assert(false);
	std::shared_ptr<Image_TGA> image = std::make_shared<Image_TGA>(width, height, colorFormat, imageData, true);
	image->setOrigin(header->xOrigin, header->yOrigin);
	return image;
}

std::shared_ptr<Image_TGA> Image_TGA::decodeFile(const std::string& path)
{
	uint32_t width = 0, height = 0;
	Color::Format colorFormat = Color::C4_U8;
	uint8_t* data = nullptr;
	
	uint8_t bytes;
	size_t length;
	std::vector<uint32_t> colorMap;
	uint16_t xOrigin = 0, yOrigin = 0;
	Descriptor descriptor = static_cast<Descriptor>(0);
	
//	std::fstream file(path);
	FILE* file = fopen(path.c_str(), "rb");
	if(!file)
	{
		slog.w(TAG, "file [%s] not found", path.c_str());
		goto bail;
	}

	Header header;
	fread(&header, sizeof(header), 1, file);

#if __BIG_ENDIAN__
	byte2swap(header.colorMapStart);
	byte2swap(header.colorMapLength);
	byte2swap(header.xOrigin);
	byte2swap(header.xOrigin);
	byte2swap(header.width);
	byte2swap(header.height);
#endif

	if(header.colorMapBits != 15 && header.colorMapBits != 16 &&
			header.colorMapBits != 24 && header.colorMapBits != 32)
	{
		slog.e(TAG, "invalid colorMapBits value(%d)", static_cast<int>(header.colorMapBits));
		goto bail;
	}

	// skip image identifier field
	if(header.idLength)
		(void)fseek(file, header.idLength, SEEK_SET);

	if(header.colorMapType == 1)  // indicates that a color-map is included with this image.
	{
		colorMap.reserve(header.colorMapLength);
		const size_t colorMapBytes = (header.colorMapBits + 7)/8;  // ceiling, make 15 return 2.
		size_t paletteSize = header.colorMapLength * colorMapBytes;
		std::vector<uint8_t> palette(paletteSize);
		fread(palette.data(), paletteSize, 1, file);

		// convert to 32 bits palette
		uint32_t* d = static_cast<uint32_t*>(colorMap.data());
		const uint8_t* s = static_cast<const uint8_t*>(palette.data());
		for(size_t i = 0; i < header.colorMapLength; ++i, ++d)
		{
			switch(header.colorMapBits)
			{
			case 15:
				*d = Color::from_RGBX5551(*reinterpret_cast<const uint16_t*>(s));
				s += 2;
				break;
			
			case 16:
				*d = Color::from_RGBA5551(*reinterpret_cast<const uint16_t*>(s));
				s += 2;
				break;
			
			case 24:
				*d = (s[0] | (s[1] << 8) | (s[2] << 16) | 0xFF000000);
				s += 3;
				break;
			
			case 32:
				*d = *reinterpret_cast<const uint32_t*>(s);
				s += 4;
				break;
			}
		}
	}

	bytes = header.depth / 8;
	switch(bytes)
	{
	case 1: colorFormat = Color::Format::C1_U8; break;
	case 2: colorFormat = Color::Format::C2_U8; break;
	case 3: colorFormat = Color::Format::C3_U8; break;
	case 4: colorFormat = Color::Format::C4_U8; break;
	default: assert(false); goto bail;          break;
	}

	width = header.width;
	height = header.height;
	length = header.width * header.height * bytes;
	data = new (std::nothrow) uint8_t[length];
	if(!data)
		goto bail;
	
	switch(header.imageType)
	{
	case TGA_NONE:
		break;
	case TGA_INDEXED:
	case TGA_INDEXED_RLE:
		break;
	case TGA_RGB:
	case TGA_RGB_RLE:
		colorFormat = (bytes == 3) ? Color::Format::C3_U8 : Color::Format::C4_U8;
		break;
	case TGA_GRAYSCALE:
	case TGA_GRAYSCALE_RLE:
		colorFormat = Color::Format::C1_U8;
		break;
	}

	// TODO
	if(header.imageType == TGA_NONE)
		slog.d(TAG, "TGA_NONE is unimplemented yet.");
	if(header.imageType == TGA_INDEXED)
		slog.d(TAG, "TGA_INDEXED is unimplemented yet.");
	else if(header.imageType == TGA_RGB)
	{
		colorFormat = Color::Format::C3_U8;
		fread(data, length, 1, file);
	}
	else if(header.imageType == TGA_GRAYSCALE)
	{
		colorFormat = Color::Format::C1_U8;
		fread(data, length, 1, file);
	}
	else if(TGA_INDEXED_RLE <= header.imageType && header.imageType <= TGA_GRAYSCALE_RLE)  // RLE compressed pixels
	{
		for(size_t i = 0; i < length;)
		{
			uint8_t ch = fgetc(file);
			if(ch >= 128)  // RLE data
			{
				uint8_t r = fgetc(file), g = fgetc(file), b = fgetc(file);
				uint8_t a = (bytes == 4)?fgetc(file):255;
//				uint32_t color = Color::rgba(r, g, b, a);

				for(uint8_t j = ch - 127; j > 0; --j)
				{
					data[i++] = r;
					data[i++] = g;
					data[i++] = b;
					if(bytes == 4)
						data[i++] = a;
				}
			}
			else  // non RLE pixels
			{
				uint32_t stride = ch + 1;
				fread(data + i, stride, 1, file);
				i += stride;
			}
		}
	}

	// post processing
/*
	if(header.descriptor & DESC_LEFT_TO_RIGHT)
		slog.v(TAG, "left to right");
	else
		slog.v(TAG, "right to left");
	if(header.descriptor & DESC_TOP_TO_BOTTOM)
		slog.v(TAG, "top to bottom");
	else
		slog.v(TAG, "bottom to top");
*/
	descriptor = header.descriptor;

bail:
	fclose(file);
	
	std::shared_ptr<Image_TGA> image = std::make_shared<Image_TGA>(width, height, colorFormat, data, true);
//	slog.d(TAG, "width=%u, height=%u, format=%d, data=%p", width, height, colorFormat, data);
	image->setOrigin(xOrigin, yOrigin);
	image->descriptor = descriptor;
	
	if((descriptor & DESC_TOP_TO_BOTTOM) != DESC_TOP_TO_BOTTOM)
	{
		slog.i(TAG, "image goes from bottom to top, flip vertically!");
		image->flipVertical();
		descriptor |= DESC_TOP_TO_BOTTOM;
		yOrigin = height - yOrigin;
	}
	
	return image;
}

bool Image_TGA::save(const std::string& path) const
{
	std::ofstream file(path, std::ios::out | std::ios::binary);
	if(!file.is_open())
	{
		slog.w(TAG, "can't open file %s for writing", path.c_str());
		return false;
	}

	Type type = TGA_NONE;
	uint8_t depth = 8;
	Descriptor descriptor = this->descriptor;  // DESC_LEFT_TO_RIGHT | DESC_TOP_TO_BOTTOM;
	switch(colorFormat)
	{
	case Color::Format::C1_U8:
		type = TGA_GRAYSCALE;
		depth = 8;
		break;
	case Color::Format::C2_U8:
		type = TGA_GRAYSCALE;
		depth = 8;
		descriptor |= DESC_ALPHA_BIT3;
		break;
	case Color::Format::RGB565_U16:
		type = TGA_RGB;
		depth = 16;
		break;
	case Color::Format::RGBA5551_U16:
		type = TGA_RGB;
		depth = 16;
		descriptor |= DESC_ALPHA_BIT0;
		break;
	case Color::Format::C3_U8:
		type = TGA_RGB;
		depth = 24;
		break;
	case Color::Format::C4_U8:
		type = TGA_RGB;
		depth = 32;
		descriptor |= DESC_ALPHA_BIT3;
		break;
	default:
		return false;
		break;
	}

	Header header =
	{
		0,  // idLength, leave it empty
		0,  // colorMapType, zero indicates colorMapStart, colorMapLength & colorMapBits should be set to zero.
		type,
		0,  // colorMapStart
		0,  // colorMapLength
		0,  // colorMapBits
		xOrigin,  // image x origin, left
		yOrigin,  // image y origin, bottom
		static_cast<uint16_t>(width),
		static_cast<uint16_t>(height),
		depth,
		descriptor
	};

	Extension extension;
	memset(&extension, 0, sizeof(extension));
	extension.size = sizeof(extension);

	std::time_t now = std::time(0);
	tm* gmtm = gmtime(&now);

	extension.stampMonth  = gmtm->tm_mon + 1;
	extension.stampDay    = gmtm->tm_mday;
	extension.stampYear   = gmtm->tm_year + 1900;
	extension.stampHour   = gmtm->tm_hour;
	extension.stampMinute = gmtm->tm_min;
	extension.stampSecond = gmtm->tm_sec;

	strncpy(extension.jobName, "C++ Programmer", sizeof(extension.jobName));

	Footer footer;
	footer.extensionOffset = 0;
	footer.developerOffset = 0;
	strncpy(footer.signature, reinterpret_cast<const char*>(&MAGIC), sizeof footer.signature);

#if __BIG_ENDIAN__
	byte2swap(header.colorMapStart);
	byte2swap(header.colorMapLength);
	byte2swap(header.xOrigin);
	byte2swap(header.xOrigin);
	byte2swap(header.width);
	byte2swap(header.height);

	byte2swap(extension.size);
	byte2swap(extension.stampMonth);
	byte2swap(extension.stampDay);
	byte2swap(extension.stampYear);
	byte2swap(extension.stampHour);
	byte2swap(extension.stampMinute);
	byte2swap(extension.stampSecond);

	byte4swap(footer.extensionOffset);
	byte4swap(footer.developerOffset);
#endif

	// write 32 bit RGBA color mode, no palette
	// 1. TGA File Header; 2. Image/ColorMap Data; 3. Developer Area; 4. Extension Area; 5. TGA File Footer.
	const size_t length = header.width * header.height * (header.depth / 8);
	file.write(reinterpret_cast<char*>(&header), sizeof(header));
	file.write(reinterpret_cast<const char*>(getData()), length);
//	file.write(reinterpret_cast<char*>(&developer), sizeof(developer));  // omit this part
	file.write(reinterpret_cast<char*>(&extension), sizeof(extension));
	file.write(reinterpret_cast<char*>(&footer), sizeof(footer));

	file.close();
	return true;
}

void Image_TGA::setOrigin(uint16_t x, uint16_t y)
{
	xOrigin = x;
	yOrigin = y;
}
