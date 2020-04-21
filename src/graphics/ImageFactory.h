#ifndef PEA_GRAPHICS_IMAGE_FACTORY_H_
#define PEA_GRAPHICS_IMAGE_FACTORY_H_

#include <memory>

#include "graphics/Image.h"

namespace pea {

class Image;

class ImageFactory
{
private:
	
public:
	ImageFactory() = delete;
	~ImageFactory() = delete;

	ImageFactory(const ImageFactory&) = delete;
	ImageFactory& operator = (const ImageFactory&) = delete;

	static Image::Format probe(const uint8_t* data, size_t length);
	static Image::Format probe(const std::string& path);
	static Image::Format probeFileName(const std::string& filename);

	static std::shared_ptr<Image> decodeFile(const std::string& path);
	static std::shared_ptr<Image> decodeByteArray(const uint8_t* data, size_t length);
//	static int32_t runLengthEncoding()

//	static Image* load(const std::string& path);
	static bool   save(const std::string& path, const Image* image);
};

}  // namespace pea
#endif  // PEA_GRAPHICS_IMAGE_FACTORY_H_
