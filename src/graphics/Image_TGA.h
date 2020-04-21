#ifndef PEA_GRAPHICS_IMAGE_TGA_H_
#define PEA_GRAPHICS_IMAGE_TGA_H_

#include <memory>

#include "graphics/Image.h"
#include "util/compiler.h"  // for packed alignment

namespace pea {

/**
 * @see https://en.wikipedia.org/wiki/Truevision_TGA
 *
 * Truevision TGA file format specification version 2.0
 * @see doc/TGA_file_format_specification.pdf
 */
class Image_TGA: public Image
{
public:
	enum Type: uint8_t
	{
		TGA_NONE          =  0,
		TGA_INDEXED       =  1,
		TGA_RGB           =  2,
		TGA_GRAYSCALE     =  3,
		TGA_INDEXED_RLE   =  9,
		TGA_RGB_RLE       = 10,
		TGA_GRAYSCALE_RLE = 11,
	};

	enum Descriptor: uint8_t
	{
		// alpha channel bits, at most 8 bits.
		DESC_ALPHA_BIT0 = 1<<0,
		DESC_ALPHA_BIT1 = 1<<1,
		DESC_ALPHA_BIT2 = 1<<2,
		DESC_ALPHA_BIT3 = 1<<3,
		DESC_ALPHA_BITS = (DESC_ALPHA_BIT3 | DESC_ALPHA_BIT2 | DESC_ALPHA_BIT1 | DESC_ALPHA_BIT0),

		// directions
		DESC_LEFT_TO_RIGHT = 1<<4,
		DESC_TOP_TO_BOTTOM = 1<<5,

		// must be zero to insure future compatibility.
		DESC_RESERVED_BIT6 = 1<<6,
		DESC_RESERVED_BIT7 = 1<<7,
	};

	PACKED_STRUCT(
	struct Header
	{
		uint8_t  idLength;       // size of ID field that follows header (0)
		uint8_t  colorMapType;   // 0 = none, 1 = palette, 2-127 reserved, 128-255 for developers
		Type     imageType;     //
		uint16_t colorMapStart;  // first color map entry
		uint16_t colorMapLength; // number of colors
		uint8_t  colorMapBits;   // bits per palette entry, typically 15, 16, 24 or 32 bits.
		uint16_t xOrigin;        // image x origin, left
		uint16_t yOrigin;        // image y origin, bottom
		uint16_t width;          // width in pixels
		uint16_t height;         // height in pixels
		uint8_t  depth;          // bits per pixel (8 16, 24, 32)
		Descriptor descriptor;   // image descriptor, alpha channel depth bits and directions.
	});

	PACKED_STRUCT(
	struct Extension
	{
		uint16_t size;                // extension size, always 495
		char     authorName[41];      // if not used, set to NULL(\0) or spaces
		char     authorComment[324];  // organized as four lines, each consisting of 80 characters plus a NULL
		uint16_t stampMonth;          // month: 1~12
		uint16_t stampDay;            // day: 1~31
		uint16_t stampYear;           // year: 0000~9999
		uint16_t stampHour;           // hour: 0~23
		uint16_t stampMinute;         // minute: 0~59
		uint16_t stampSecond;         // second: 0~59
		char     jobName[41];         // job name/id
		uint16_t jobHour;             // job time: hours
		uint16_t jobMinute;           // job time: minutes
		uint16_t jobSecond;           // job time: seconds
		char     softwareId[41];      // the application that created the file.
		uint16_t versionNumber;       // software version number
		uint8_t  versionLetter;       // software version letter
		uint32_t keyColor;            // key color
		uint16_t pixelNumerator;      // pixel aspect ratio
		uint16_t pixelDenominator;    // pixel aspect ratio
		uint16_t gammaNumerator;      // gamma value
		uint16_t gammaDenominator;    // gamma value
		uint32_t colorOffset;         // color correction offset
		uint32_t stampOffset;         // postage stamp offset
		uint32_t scanOffset;          // scan-line table offset
		uint8_t  attributesType;      // attributes types
	});

	PACKED_STRUCT(
	struct Footer
	{
		uint32_t extensionOffset;     //
		uint32_t developerOffset;     //
		char     signature[18];       // "TRUEVISION-XFILE."
	});

private:
	uint16_t xOrigin, yOrigin;
	Descriptor descriptor;

public:
	Image_TGA();
	Image_TGA(uint32_t width, uint32_t height, Color::Format format, uint8_t* data);
	
	virtual ~Image_TGA();
	
	Format getImageFormat() const override { return Format::TGA; }
	
	static bool probe(const uint8_t* data, size_t length);

	static std::shared_ptr<Image_TGA> decodeByteArray(const uint8_t* data, size_t length);
	static std::shared_ptr<Image_TGA> decodeFile(const std::string& path);
	
	virtual bool save(const std::string& path) const override;

	void setOrigin(uint16_t x, uint16_t y);
};

// overload bitwise operators for enum class Descriptor
inline Image_TGA::Descriptor operator |(const Image_TGA::Descriptor& lhs, const Image_TGA::Descriptor& rhs)
{
	return static_cast<Image_TGA::Descriptor>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}

inline Image_TGA::Descriptor& operator |= (Image_TGA::Descriptor& lhs, const Image_TGA::Descriptor& rhs)
{
	*reinterpret_cast<uint8_t*>(&lhs) |= *reinterpret_cast<const uint8_t*>(&rhs);
	return lhs;
}

}  // namespace pea
#endif  // PEA_GRAPHICS_IMAGE_TGA_H_
