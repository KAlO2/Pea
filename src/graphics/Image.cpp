#include "graphics/Image.h"

#include <vector>
#include <cstring>  // for memcpy

#include "pea/config.h"
#if OpenMP_CXX_FOUND
#include <omp.h>
#endif

#include "graphics/Image_BMP.h"
#include "graphics/Image_JPG.h"
#include "graphics/Image_PNG.h"
#include "util/Log.h"
#include "util/utility.h"


static const char* TAG = "Image";

using namespace pea;


void Image::convert_RGBX5551_to_RGB888(const uint8_t* src, uint8_t* dst, size_t count)
{
	const uint16_t* s = reinterpret_cast<const uint16_t*>(src);
	
	#pragma omp parallel for
	for(size_t i = 0; i < count; ++i)
	{
		*dst = (*s & 0x001F) << 3;  ++dst;
		*dst = (*s & 0x03E0) << 3;  ++dst;
		*dst = (*s & 0x7C00) << 3;  ++dst;
		++s;
	}
}

void Image::convert_RGB565_to_RGB888(const uint8_t* src, uint8_t* dst, size_t count)
{
	const uint16_t* s = reinterpret_cast<const uint16_t*>(src);
	
	#pragma omp parallel for
	for(size_t i = 0; i < count; ++i)
	{
		*dst = (*s & 0x001F) << 3;  ++dst;
		*dst = (*s & 0x07E0) << 2;  ++dst;
		*dst = (*s & 0xF800) << 3;  ++dst;
		++s;
	}
}

void Image::convert_RGB565_to_RGBA8888(const uint8_t* src, uint8_t* dst, size_t count)
{
	const uint16_t* s = reinterpret_cast<const uint16_t*>(src);
	uint32_t* d = reinterpret_cast<uint32_t*>(dst);
	
	#pragma omp parallel for
	for(size_t i = 0; i < count; ++i)
	{
//		*d = Color::from_RGB565(*s);
		*d = ((*s & 0x001F) << 3) | ((*s & 0x07E0) << 5) | (*s & 0xF800) << 8 | 0xFF000000;
		++s;
		++d;
	}
}

void Image::convert_RGBA5551_to_RGBA8888(const uint8_t* src, uint8_t* dst, size_t count)
{
	const uint16_t* s = reinterpret_cast<const uint16_t*>(src);
	uint32_t* d = reinterpret_cast<uint32_t*>(dst);
	
	#pragma omp parallel for
	for(size_t i = 0; i < count; ++i)
	{
//		*d = Color::fromRGBA_5551(*s);
		*dst = (*s & 0x001F) << 3;  ++dst;
		*dst = (*s & 0x03E0) << 3;  ++dst;
		*dst = (*s & 0x7C00) << 3;  ++dst;
		++s;
		++d;
	}
}

void Image::convert_RGB888_to_RGBA8888(const uint8_t* src, uint8_t* dst, size_t count)
{
	uint32_t* d = reinterpret_cast<uint32_t*>(dst);
	
	#pragma omp parallel for
	for(size_t i = 0; i < count; ++i)
	{
		*d = src[0] | (src[1] << 8) | (src[2] << 16) | 0xFF000000;
		src += 3;
		++d;
	}
}

Image::Image():
		width(0),
		height(0),
		colorFormat(Color::RGBA_8888),
		data(nullptr)
{
}

Image::Image(uint32_t width, uint32_t height, Color::Format format) noexcept(false):
		width(width),
		height(height),
		colorFormat(format),
		data(new uint8_t[width * height * Color::size(format)])
{
	if(data == nullptr)  // std::bad_alloc
		width = height = 0;
}

Image::Image(uint32_t width, uint32_t height, Color::Format format, uint8_t* data):
		width(width),
		height(height),
		colorFormat(format),
		data(data)
{
	
}

Image::~Image()
{
//	if(m_palette)
//	{
//		SAFE_DELETE(m_index);
//		delete m_palette;
//	}
//	else
	SAFE_DELETE_ARRAY(data);
}

bool Image::isValid() const
{
	return width > 0 && height > 0 && colorFormat != Color::UNKNOWN && data != nullptr;
}

void Image::setPixel(uint32_t x, uint32_t y, uint32_t color)
{
	assert(x < width && y < height);
	assert(data != nullptr);

	int32_t offset = (y * width + x) * Color::size(colorFormat);
	uint8_t* p = data + offset;

	switch(colorFormat)
	{
	case Color::G_8:
		*p = Color::to_G8(color);
		break;
		
	case Color::GA_88:
		*reinterpret_cast<uint16_t*>(p) = Color::to_GA88(color);
		break;
		
	case Color::RGBA_5551:
		*reinterpret_cast<uint16_t*>(p) = Color::to_RGBA5551(color);
		break;
		
	case Color::RGB_565:
		*reinterpret_cast<uint16_t*>(p) = Color::to_RGB565(color);
		break;
		
	case Color::RGB_888:
		p[0] = color & 0xFF;
		p[1] = (color & 0xFF00) >> 8;
		p[2] = (color & 0xFF00) >> 8;
		break;
	
	case Color::RGBA_8888:
		*reinterpret_cast<uint32_t*>(p) = color;
		break;
	
	default:
		assert(false);  // you should not be here.
		break;
	}
}

uint32_t Image::getPixel(uint32_t x, uint32_t y) const
{
	assert(x < width && y < height);
	assert(data != nullptr);
//	if(m_isCompressed)
//	{
//		slog.w(TAG, "Image::getPixel function doesn't work with compressed images.");
//		return color;
//	}

	int32_t offset = (y * width + x) * Color::size(colorFormat);
	const uint8_t* p = data + offset;

	switch(colorFormat)
	{
	case Color::G_8:
		return Color::from_G8(*p);
	
	case Color::GA_88:
		return Color::from_GA88(*reinterpret_cast<const uint16_t*>(p));
	
	case Color::RGBA_5551:
		return Color::from_RGBA5551(*reinterpret_cast<const uint16_t*>(p));
	
	case Color::RGB_565:
		return Color::from_RGB565(*reinterpret_cast<const uint16_t*>(p));
	
	case Color::RGB_888:
		return p[0] | (p[1] << 8) | (p[2] << 16) | 0xFF000000;
	
	case Color::RGBA_8888:
		return *reinterpret_cast<const uint32_t*>(p);
		
	default:
		assert(false);
		return 0;  // you should not be here.
		break;
	}
}

void Image::fillColor(uint32_t color)
{
	for(uint32_t y = 0; y < height; ++y)
		for(uint32_t x = 0; x < width; ++x)
			setPixel(x, y, color);
}

void Image::fillCheckerboard(uint32_t size, uint32_t color1/* = 0xFF000000 */, uint32_t color2/* = 0xFFFFFFFF */)
{
	assert(size > 0u);
	for(uint32_t y = 0; y < height; ++y)
	{
		uint32_t block_r = y / size;
		for(uint32_t x = 0; x < width; ++x)
		{
			uint32_t block_c = x / size;
			const uint32_t& color = (block_r + block_c) % 2 != 0 ? color1 : color2;
			setPixel(x, y, color);
		}
	}
}

#if 0
Image Image::crop(const Rect& rect, uint32_t backgroundColor/* = 0*/) const
{
	if(rect.isEmpty())
		return {};
	
	uint32_t dstWidth = rect.getWidth();
	uint32_t dstHeight = rect.getHeight();
	
	Image image(dstWidth, dstHeight, colorFormat);
	uint8_t* dstData = image.getData();
	
	if(dstWidth > width || dstHeight > height)
		image.fillColor(backgroundColor);
	
	uint32_t h0 = rect.top >= 0 ? rect.top: 0U;
	uint32_t w0 = rect.left >= 0 ? rect.left: 0U;
	uint32_t h1 = rect.bottom < static_cast<int32_t>(height)? rect.bottom: height;
	uint32_t w1 = rect.right < static_cast<int32_t>(right)? rect.right: right;
	uint32_t pixelSize = Color::size(colorFormat);
	const uint32_t srcRowStride = width * pixelSize;
	const uint32_t dstRowStride = dstWidth * pixelSize;
	
	uint8_t* src = data + h0 * srcRowStride + w0 * pixelSize;
	uint8_t* dst = image.getData() + (h0 - rect.top) *dstRowStride + (w0 - rect.left) * pixelSize;
	for(uint32_t h = h0; h < h1; ++h)
	{
		std::memcpy(dst, src, rowLength);
		src += srcRowStride;
		dst += dstRowStride;
	}
	
	return image;
}
#endif
void Image::flipHorizontal()
{
	for(uint32_t h = 0; h < height; ++h)
	{
		for(uint32_t w1 = 0, w2 = width - 1; w1 < w2; ++w1, --w2)
		{
			uint32_t c1 = getPixel(w1, h);
			uint32_t c2 = getPixel(w2, h);
			setPixel(w1, h, c2);
			setPixel(w2, h, c1);
		}
	}
}

void Image::flipVertical()
{
	size_t rowStride = width * Color::size(colorFormat);
	std::vector<uint8_t> line(rowStride);
	
	uint8_t* src = data;
	uint8_t* dst = data + (height - 1) * rowStride;
	uint8_t* tmp = line.data();
	for(uint32_t i = 0; i < height / 2; ++i)
	{
		std::memcpy(tmp, src, rowStride);
		std::memcpy(src, dst, rowStride);
		std::memcpy(dst, tmp, rowStride);
		
		src += rowStride;
		dst -= rowStride;
	}
}

