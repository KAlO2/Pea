#ifndef PEA_IO_IMAGE_JPG_H_
#define PEA_IO_IMAGE_JPG_H_

#include <memory>

#include "graphics/Image.h"

namespace pea {

class Image_JPG: public Image
{
private:
	int32_t quality;  // percentage, that's in (0, 100].

public:
	Image_JPG();
	Image_JPG(int32_t width, int32_t height, uint8_t* data, bool move);
	virtual ~Image_JPG();

	Format getImageFormat() const override { return Format::JPG; }
	
	void setQuality(int32_t quality);
	int32_t getQuality() const;

	static bool probe(const uint8_t* data, size_t length);
	
	static std::shared_ptr<Image_JPG> decodeByteArray(const uint8_t* data, size_t length);
	static std::shared_ptr<Image_JPG> decodeFile(const std::string& path);
	
//	virtual bool load(const uint8_t* data, size_t length) override;
//	virtual bool load(const std::string& filename) override;
	bool save(const std::string& filename) const override;


};

}  // namespace pea
#endif  // PEA_IO_IMAGE_JPG_H_
