#ifndef PEA_GRAPHICS_IMAGE_BMP_H_
#define PEA_GRAPHICS_IMAGE_BMP_H_

#include <memory>

#include "graphics/Image.h"
#include "util/compiler.h"  // for packed alignment

namespace pea {

/**
 * Some useful links about BMP file specification
 * Microsoft Windows Bitmap File Format Summary @see http://www.fileformat.info/format/bmp/egff.htm
 * BMP file format @see http://en.wikipedia.org/wiki/BMP_file_format and external links listed below.
 */
class Image_BMP: public Image
{
private:
	// http://msdn.microsoft.com/en-us/library/windows/desktop/dd183374(v=vs.85).aspx
	PACKED_STRUCT(
	struct BITMAP_FILE_HEADER
	{
		uint16_t type;
		uint32_t size;
		uint16_t reserved1;
		uint16_t reserved2;
		uint32_t offBits;
	});

	// http://msdn.microsoft.com/en-us/library/windows/desktop/dd183376(v=vs.85).aspx
	PACKED_STRUCT(
	struct BITMAP_INFO_HEADER
	{
		uint32_t size;
		 int32_t width;
		 int32_t height;
		uint16_t planes;
		uint16_t bitCount;
		uint32_t compression;
		uint32_t sizeImage;
		 int32_t xPixelsPerMeter;
		 int32_t yPixelsPerMeter;
		uint32_t colorUsed;
		uint32_t colorImportant;
	});

	PACKED_STRUCT(
	struct BITMAP_COLOR_MASK
	{
		uint32_t redMask;
		uint32_t greenMask;
		uint32_t blueMask;
		uint32_t alphaMask;
	});

	PACKED_STRUCT(
	struct BITMAP_V4_HEADER
	{
		BITMAP_INFO_HEADER info_header;
		BITMAP_COLOR_MASK  color_mask;
		uint32_t colorSpaceType;
		uint32_t endpoints[9]; // CIEXYZTRIPLE Endpoints;
		uint32_t gammaRed;
		uint32_t gammaGreen;
		uint32_t gammaBlue;
	});

	PACKED_STRUCT(
	struct BITMAP_V5_HEADER
	{
		BITMAP_V4_HEADER v4_header;

		uint32_t intent;
		uint32_t profileData;
		uint32_t profileSize;
		uint32_t reserved;
	});

	enum CompressionMethod
	{
		COMPRESSION_RGB = 0,  // most common
		COMPRESSION_RLE8,     // A run-length encoded (RLE) format for bitmaps with 8 BPP
		COMPRESSION_RLE4,     // An RLE format for bitmaps with 4 BPP (Bits Per Pixel)
		COMPRESSION_BITFIELDS,// can be used only with 16 and 32 BPP
		COMPRESSION_JPEG,     // the bitmap contains a JPEG image
		COMPRESSION_PNG,      // the bitmap contains a PNG image
	};

	PACKED_STRUCT(
	struct BITMAP_INFO
	{
		BITMAP_INFO_HEADER info_header;
		Color *table;
	});

private:
	Color *table;
	bool palette;

private:
	

public:
	Image_BMP();
	Image_BMP(uint32_t width, uint32_t height, Color::Format format);
	Image_BMP(uint32_t width, uint32_t height, Color::Format format, uint8_t* data, bool move);
	virtual ~Image_BMP();
	
	Format getImageFormat() const override { return Format::BMP; }
	
	static bool probe(const uint8_t* data, size_t length);
	
	static std::shared_ptr<Image_BMP> decodeByteArray(const uint8_t* data, size_t length);
	static std::shared_ptr<Image_BMP> decodeFile(const std::string& path);
	
	virtual bool save(const std::string& path) const override;


};

}  // namespace pea
#endif  // PEA_GRAPHICS_IMAGE_BMP_H_
