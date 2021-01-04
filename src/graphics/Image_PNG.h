#ifndef PEA_GRAPHICS_IMAGE_PNG_H_
#define PEA_GRAPHICS_IMAGE_PNG_H_

#include <memory>

#include "graphics/Image.h"

namespace pea {

class Image_PNG: public Image
{
private:
//	std::map<const std::string, std::string> comments;
//	http://zarb.org/~gc/html/libpng.html
public:
	Image_PNG();
	Image_PNG(uint32_t width, uint32_t height, Color::Format format);
	Image_PNG(uint32_t width, uint32_t height, Color::Format format, uint8_t*& data, bool move);
	virtual ~Image_PNG();
	
	Format getImageFormat() const override { return Format::PNG; }

	static bool probe(const uint8_t* data, size_t length);
	
	static std::shared_ptr<Image_PNG> decodeByteArray(const uint8_t* data, size_t length);
	static std::shared_ptr<Image_PNG> decodeFile(const std::string& path);

	bool save(const std::string& path) const override;

};

}  // namespace pea
#endif  // PEA_GRAPHICS_IMAGE_PNG_H_
