#include "graphics/Image_JPG.h"

#include "util/Log.h"
#include "math/scalar.h" // for clamp

#include <jpeglib.h>

static const char* TAG = "Image_JPG";

using namespace pea;

static const uint8_t MAGIC[2] = {0xFF, 0xD8};

Image_JPG::Image_JPG():
		Image_JPG(0, 0, nullptr)
{
	
}

Image_JPG::Image_JPG(int32_t width, int32_t height, uint8_t* data):
		Image(width, height, Color::RGB_888, data),
		quality(92)
{
	// JPEG only support RGB
}

Image_JPG::~Image_JPG()
{
	// TODO Auto-generated destructor stub
}

void Image_JPG::setQuality(int quality)
{
	// Safety limit on quality factor. Convert 0 to 1 to avoid zero divide.
	int clamped = clamp(quality, 1, 100);
	if(clamped != quality)
		slog.w(TAG, "bad quality value: %d, out of range [1, 100]", quality);
	this->quality = clamped;
}

int Image_JPG::getQuality() const
{
	return quality;
}

bool Image_JPG::probe(const uint8_t* data, size_t length)
{
	if(length <= 8)  // TODO: shortest JPG file length
		return false;
	
	return *reinterpret_cast<const uint16_t*>(data) ==
	       *reinterpret_cast<const uint16_t*>(MAGIC);
}

std::shared_ptr<Image_JPG> Image_JPG::decodeByteArray(const uint8_t* data, size_t length)
{
	std::shared_ptr<Image_JPG> image;
	
	// This struct contains the JPEG decompression parameters and pointers to
	// working space (which is allocated as needed by the JPEG library).
	jpeg_decompress_struct cinfo;

	// Note that this struct must live as long as the main JPEG parameter struct
	// to avoid dangling-pointer problems.
	jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	// libjpeg data structure for storing one row, that is, scanline of an image
	JSAMPROW row_pointer[1] = {0};
	unsigned long location = 0;
	// TODO

	bool flag = false;
	do
	{
		break;
		flag = true;
	}while(false);

	
	return image;
}

std::shared_ptr<Image_JPG> Image_JPG::decodeFile(const std::string& path)
{
	std::shared_ptr<Image_JPG> image;
	FILE *file = fopen(path.c_str(), "rb");
	if(!file)
	{
		slog.w(TAG, "can't open file %s for reading", path.c_str());
		return image;
	}

	jpeg_decompress_struct cinfo; // a JPEG object
	jpeg_error_mgr jerr;          // a JPEG error handler

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, file);

	(void)jpeg_read_header(&cinfo, true/*requre_image*/);
	// Ignore the return value from jpeg_read_header since
	//   (a) suspension is not possible with the stdio data source, and
	//   (b) we passed true to reject a tables-only JPEG file as an error.
	// See libjpeg.doc for more info.

	// set parameters for decompression

	// start decompressor
	(void)jpeg_start_decompress(&cinfo);

	// default no scaling, cinfo.scale_num = cinfo.scale_denom = 1
	int row_stride = cinfo.output_width * cinfo.output_components;
	uint8_t* data = new uint8_t[cinfo.output_height * row_stride];
	JSAMPROW buffer = data;
	if(!data)
	{
		slog.e(TAG, "allocate memory failed");
		goto bail;
	}

	image = std::make_shared<Image_JPG>(cinfo.output_width, cinfo.output_height, data);
	
	while(cinfo.output_scanline < cinfo.output_height)
	{
		(void)jpeg_read_scanlines(&cinfo, &buffer, 1);
		buffer += row_stride;
	}

	(void)jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
bail:
	fclose(file);
	return image;
}

bool Image_JPG::save(const std::string& filename) const
{
	FILE *file = fopen(filename.c_str(), "wb");
	if(!file)
	{
		slog.w(TAG, "can't open file %s for writing", filename.c_str());
		return false;
	}

	jpeg_compress_struct cinfo;  // a JPEG object
	jpeg_error_mgr jerr;         // a JPEG error handler

	// Allocate and initialize JPEG compression object
	// Set up the error handler first, in case the initialization step fails.
	// (Unlikely, but it could happen if you are out of memory.)
	// This routine fills in the contents of struct jerr, and returns jerr's
	// address which we place into the link field in cinfo.
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, file);

	// give some specifications about the image to save to libjpeg
	// set parameters for compression
	cinfo.image_width = this->width;
	cinfo.image_height = this->height;
	cinfo.input_components = 3;      // color components per pixel, 3 for RGB
	cinfo.in_color_space = JCS_RGB;  // colorspace of input image

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, true/*force_baseline*/);
	jpeg_start_compress(&cinfo, true/*write_all_tables*/);

	// while (scan lines remain to be written)
	const int row_stride = this->width * 3;  // JSAMPLEs per row width in image buffer
	JSAMPROW row_pointer[1];  // pointer to a single row
	while(cinfo.next_scanline < this->height)
	{
		row_pointer[0] = &data[cinfo.next_scanline * row_stride];
		(void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	// Step 6: finish compression
	jpeg_finish_compress(&cinfo);
	fclose(file);

	// Step 7: release JPEG compression object
	jpeg_destroy_compress(&cinfo);

	return true;
}
