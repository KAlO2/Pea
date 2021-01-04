#ifndef PEA_IO_IMAGE_H_
#define PEA_IO_IMAGE_H_

#include <string>

#include "graphics/Color.h"
//#include "graphics/Rect.h"

namespace pea {

/**
 * @class Image
 * Image is expressed in 2D matrix in the memory.
 * compressed image will be decompressed into memory.
 * indexed image will recover back RGB/RGBA image, and each channel has 8 bits.
 * origin(0, 0) will be the top left point.
 */
class Image
{
public:
	// Note when you change the order below, ImageFactory::probe should be changed too.
	enum class Format
	{
		UNKNOWN = -1,
		PNG,
		TGA,
		JPG,
		BMP,
		TIFF,

		COUNT,  // internal use
	};

/*
	+--  --+  |__  +-----     |  |  -----+     |  |  |
	|__  __|  |    |  |    ------+    |  |   --|  +------
	|      |  +--                            --+
*/
	enum class Transform
	{
		NONE = 0,    // no transformation
		FLIP_H,      // flip horizontally
		FLIP_V,      // flip vertically
		TRANSPOSE,   // transpose across TL-to-BR line
		TRANSVERSE,  // transpose across TR-to-BL line
		ROTATE_90,   // 90 degrees counter clockwise rotation
		ROTATE_180,  // 180 degrees counter clockwise rotation
		ROTATE_270,  // 270 degrees counter clockwise rotation
	};

protected:
	uint32_t  width;
	uint32_t  height;
	Color::Format colorFormat;
	uint8_t* data;
	bool move;
public:
	static void convert_RGBX5551_to_RGB888(const uint8_t* src, uint8_t* dst, size_t count);
	static void convert_RGB565_to_RGB888(const uint8_t* src, uint8_t* dst, size_t count);
	static void convert_RGB565_to_RGBA8888(const uint8_t* src, uint8_t* dst, size_t count);
	
	static void convert_RGBA5551_to_RGBA8888(const uint8_t* src, uint8_t* dst, size_t count);
	static void convert_RGB888_to_RGBA8888(const uint8_t* src, uint8_t* dst, size_t count);
	
public:
	/**
	 * default empty image.
	 */
	Image();
	
	Image(uint32_t width, uint32_t height, Color::Format format) noexcept(false);
	
	/**
	 * @param[in] width  Image width
	 * @param[in] height Image height
	 * @param[in] format Image color format
	 * @param[in] data   it serves as unique_ptr, namely transfer ownership to this image.
	 */
	Image(uint32_t width, uint32_t height, Color::Format format, uint8_t* data, bool move = true);
	
	virtual ~Image();

	Image(const Image& other) = delete;
	Image& operator =(const Image& other) = delete;
	
	virtual Format getImageFormat() const { return Format::UNKNOWN; }
/*
	virtual bool load(const uint8_t* data, size_t length) = 0;
	virtual bool load(const std::string& filename) = 0;
	
	void load(int32_t width, int32_t height, Color::Format format, uint8_t* data);
	void save(Image::Format format, const std::string& filename);
*/
	virtual bool save(const std::string& path) const = 0;
	
	bool isValid() const;
	
	const uint8_t* getData() const { return data; }
	      uint8_t* getData()       { return data; }
	
	int32_t getWidth() const  { return width;  }
	int32_t getHeight() const { return height; }

	void setColorFormat(const Color::Format& format) { this->colorFormat = format; }
	Color::Format getColorFormat() const { return colorFormat; }

	/**
	 * set or modify a pixel in the image.
	 *
	 * @param position Zero indexed, count from top to bottom, left to right.
	 * @param color the specified color.
	 */
	void setPixel(uint32_t x, uint32_t y, uint32_t color);

	uint32_t getPixel(uint32_t x, uint32_t y) const;

	void fillColor(uint32_t color);
	
	/**
	 * @param[in] size edge length of a black/white cell
	 * @param[in] color1 default to black color
	 * @param[in] color2 default to white color
	 */
	void fillCheckerboard(uint32_t size, uint32_t color1 = 0xFF000000, uint32_t color2 = 0xFFFFFFFF);
	
	void convert(Image::Format newFormat);

	/**
	 * Crop the image to a specific size.
	 * If larger, then the image is filled with black color by default.
	 * @param[in] rect the region to be cropped.
	 * @param[in] backgroundColor color to fill if larger than the source image.
	 */
//	Image crop(const Rect& rect, uint32_t backgroundColor = 0) const;

	void flipHorizontal();
	void flipVertical();
};

}  // namespace pea
#endif  // PEA_IO_IMAGE_H_
