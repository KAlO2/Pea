#include "graphics/Image_PNG.h"

#include <fstream>
#include <string.h>
#include <vector>

#include <png.h>

#include "util/Log.h"
#include "util/utility.h"

static const char* TAG = "Image_PNG";

using namespace pea;

static const uint8_t MAGIC[8] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};

// http://www.libpng.org/pub/png/spec/1.2/PNG-Chunks.html

static void error_fn(png_structp png_ptr, png_const_charp message) {
	slog.e(TAG, "png error %s\n", message);
	slog.e(TAG, "Compiled with libpng %s; using libpng %s.\n", PNG_LIBPNG_VER_STRING, png_libpng_ver);
//	slog.e(TAG, "Compiled with zlib %s; using zlib %s.\n", ZLIB_VERSION, zlib_version);
//	longjmp(PNG_JMPBUF(png_ptr), kPngError);
}

void warning_fn(png_structp, png_const_charp message) {
	slog.e(TAG, "png warning %s\n", message);
}

Image_PNG::Image_PNG():
		Image()
{
	
}

Image_PNG::Image_PNG(uint32_t width, uint32_t height, Color::Format format):
		Image(width, height, format)
{
}

Image_PNG::Image_PNG(uint32_t width, uint32_t height, Color::Format format, uint8_t* data):
		Image(width, height, format, data)
{
}

Image_PNG::~Image_PNG()
{
	// TODO: Auto-generated destructor stub
}

bool Image_PNG::probe(const uint8_t* data, size_t length)
{
	if(length <= sizeof(MAGIC))
		return false;

	return memcmp(data, MAGIC, sizeof(MAGIC)) == 0;
}

std::shared_ptr<Image_PNG> Image_PNG::decodeByteArray(const uint8_t* data, size_t length)
{
	uint32_t width = 0, height =0;
	Color::Format colorFormat = Color::RGBA_8888;
	uint8_t* imageData = nullptr;
	
	assert(probe(data, length));
	data += sizeofArray(MAGIC);

	png_struct* png_ptr   = nullptr;
	png_info*   info_ptr  = nullptr;

	do
	{
		// png_create_read_struct(user_png_ver, error_ptr, error_fn, warn_fn))
		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
		if(!png_ptr)
			break;

		info_ptr = png_create_info_struct(png_ptr);
		if(!info_ptr)
			break;

		if(setjmp(png_jmpbuf(png_ptr)))
		{
			png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
			break;
		}

		png_set_read_fn(png_ptr, nullptr, nullptr);
//		png_set_read_fn(png_ptr, &image_source, pngReadFunction);
		png_read_info(png_ptr, info_ptr);

		int bit_depth, color_type, interlace_type;
		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, nullptr, nullptr);

		// strip 16 bits color files down to 8 bits color.
		// Use accurate scaling if it's available, otherwise just chop off the low byte.
		if(bit_depth == 16)
#ifdef PNG_READ_SCALE_16_TO_8_SUPPORTED
			png_set_scale_16(png_ptr);
#else
			png_set_strip_16(png_ptr);
#endif

		// Extract multiple pixels with bit depths of 1, 2, and 4 from a single
		// byte into separate bytes (useful for paletted and grayscale images).
		if(bit_depth < 8)
			png_set_packing(png_ptr);

		// Expand paletted colors into true RGB triplets
		if(color_type == PNG_COLOR_TYPE_PALETTE)
			png_set_palette_to_rgb(png_ptr);

		// low-bit-depth grayscale images are to be expanded to 8 bits
		if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
			png_set_expand_gray_1_2_4_to_8(png_ptr);

		// Expand paletted or RGB images with transparency to full alpha channels
		// so the data will be available as RGBA quartets.
		if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) != 0)
			png_set_tRNS_to_alpha(png_ptr);

		// Optional call to gamma correct and add the background to the palette
		// and update info structure.  REQUIRED if you are expecting libpng to
		// update the palette for you (ie you selected such a transform above).
		png_read_update_info(png_ptr, info_ptr);

		bit_depth = png_get_bit_depth(png_ptr, info_ptr);
		color_type = png_get_color_type(png_ptr, info_ptr);

		switch(color_type)
		{
		case PNG_COLOR_TYPE_GRAY:
			colorFormat = Color::Format::G_8;
			break;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			colorFormat = Color::Format::GA_88;
			break;
		case PNG_COLOR_TYPE_RGB:
			colorFormat = Color::Format::RGB_888;
			break;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			colorFormat = Color::Format::RGBA_8888;
			break;
		default:
			colorFormat = Color::Format::UNKNOWN;
			slog.e(TAG, "unknown color type (%d)", color_type);
//			assert(false);
			break;
		}

		// read png data
		png_size_t row_bytes = png_get_rowbytes(png_ptr, info_ptr);
		const png_byte** row_pointers = new const png_byte* [height];
		if(!row_pointers)
			break;

		for(png_uint_32 i = 0; i < height; ++i)
			row_pointers[i] = data + i*row_bytes;

		imageData = new (std::nothrow) uint8_t[height * row_bytes];
		if(!imageData)
		{
			delete[] row_pointers;
			break;
		}

		png_read_image(png_ptr, const_cast<png_byte**>(row_pointers));
		png_read_end(png_ptr, nullptr);

		// pre-multiplied alpha for RGBA8888
//		if(color_type == PNG_COLOR_TYPE_RGB_ALPHA)
//			premultipliedAlpha();
//		else
//			_hasPremultipliedAlpha = false;

		delete[] row_pointers;
	} while(false);

	return std::make_shared<Image_PNG>(width, height, colorFormat, imageData);
}

std::shared_ptr<Image_PNG> Image_PNG::decodeFile(const std::string& path)
{
	uint32_t width = 0, height = 0, rowStride = 0;
	Color::Format colorFormat = Color::RGBA_8888;
	uint8_t* data = nullptr;
#if 0
	png_image image;  // The control structure used by libpng
	std::memset(&image, 0, (sizeof image));
	image.version = PNG_IMAGE_VERSION;

	if(png_image_begin_read_from_file(&image, path.c_str()) != 0)
	{
		image.format = PNG_FORMAT_RGBA;
		data = new (std::nothrow) uint8_t[PNG_IMAGE_SIZE(image)];
/*
	'background' is not necessary when reading the image, because the alpha channel is preserved; 
	if it were to be removed, for example if we requested PNG_FORMAT_RGB, then either a solid 
	background color would have to be supplied, or the output buffer would have to be initialized to
	the actual background of the image.
	
	'row_stride' is the number of components allocated for the image in each row. It has to be 
	at least as big as the value returned by PNG_IMAGE_ROW_STRIDE, but if you just allocate space 
	for the default, minimum size, using PNG_IMAGE_SIZE as above, you can pass zero.
	
	'colormap' have exactly the same format as a row of image pixels (so you choose what format 
	to make the colormap by setting image.format). A colormap is only returned if
	PNG_FORMAT_FLAG_COLORMAP is also set in image.format, so in this case NULL is passed as the 
	final argument. If you do want to force all images into an index/color-mapped format, then you 
	can use PNG_IMAGE_COLORMAP_SIZE(image) to find the maximum size of the colormap in bytes.
*/
		png_const_color* background = nullptr;
		void* colormap = nullptr;
		if(data != nullptr)
		{
//			rowStride = PNG_IMAGE_ROW_STRIDE(image);
			if(png_image_finish_read(&image, background, data, rowStride, colormap) != 0)
				return make_shared<Image_PNG>(image.width, image.height, colorFormat, data);
		}
		else
		{
			png_image_free(&image);
		}
	}
	else
	{
/*
	Something went wrong reading or writing the image.
	libpng stores a textual message in the 'png_image' structure:
*/
		slog.e(TAG, "pngtopng: error: %s\n", image.message);
	}
	
#endif
	
	FILE *file = fopen(path.c_str(), "rb");
	if(!file)
	{
		slog.w(TAG, "can't open file %s for writing", path.c_str());
		return std::make_shared<Image_PNG>();
	}

	png_struct* png_ptr = nullptr;
	png_info* info_ptr = nullptr;
	
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
			nullptr,  // user_error_ptr
			nullptr,  // user_error_fn
			nullptr); // user_warning_fn
	if(!png_ptr)
	{
		slog.e(TAG, "png_create_read_struct returned 0.");
		goto bail;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr || setjmp(png_jmpbuf(png_ptr)))
	{
		slog.e(TAG, "png_create_read_struct returned 0 or set error handling failed.");
		png_destroy_write_struct(&png_ptr, (info_ptr == nullptr)? nullptr:&info_ptr);
		goto bail;
	}

	png_init_io(png_ptr, file);
	png_read_info(png_ptr, info_ptr);

	int bit_depth, color_type, interlace_type;
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, nullptr, nullptr);

//	png_set_strip_alpha(png_ptr);  // Strip alpha bytes
//	png_set_packing(png_ptr);  // pack 1, 2, and 4 BPP into bytes, useful for paletted and grayscale images
	switch(color_type)
	{
	case PNG_COLOR_TYPE_GRAY:
		if(bit_depth < 8)  // Expand grayscale images to the full 8 bits from 1, 2, or 4 BPP
			png_set_expand_gray_1_2_4_to_8(png_ptr);
		colorFormat = Color::G_8;
		break;
	case PNG_COLOR_TYPE_PALETTE:  // Expand paletted colors into true RGB triplets
		png_set_palette_to_rgb(png_ptr);
		colorFormat = Color::G_8;  //FIXME does palette have alpha?
		break;
	case PNG_COLOR_TYPE_RGB:
		colorFormat = Color::RGB_888;
		break;
	case PNG_COLOR_TYPE_RGBA:
		colorFormat = Color::RGBA_8888;
		break;
	case PNG_COLOR_TYPE_GRAY_ALPHA:
		colorFormat = Color::GA_88;
		break;
	default:
		colorFormat = Color::UNKNOWN;
		slog.d(TAG, "unknown color type: %d", color_type);
		break;
	}

	// We use png_read_image and rely on that for interlace handling, but we also
	// call png_read_update_info therefore must turn on interlace handling now:
	(void)png_set_interlace_handling(png_ptr);

	// Optional call to gamma correct and add the background to the palette
	// and update info structure.  REQUIRED if you are expecting libpng to
	// update the palette for you (i.e., you selected such a transform above).
	png_read_update_info(png_ptr, info_ptr);

	png_free_data(png_ptr, info_ptr, PNG_FREE_ROWS, 0);

	rowStride = width * Color::size(colorFormat);  // FIXME enum class
	data = new (std::nothrow) uint8_t[height * rowStride];
	if(data)
	{
		png_byte** row_pointers = reinterpret_cast<png_byte**>(png_malloc(png_ptr, height * sizeof(png_byte*)));
		for(png_uint_32 row = 0; row < height; ++row)
			row_pointers[row] = data + rowStride * row;
#ifdef PNG_INFO_IMAGE_SUPPORTED
		png_set_rows(png_ptr, info_ptr, row_pointers);
#endif

		png_read_image(png_ptr, row_pointers);

		// Read rest of file, and get additional chunks in info_ptr
		png_read_end(png_ptr, info_ptr);
	}
	else
		slog.e(TAG, "line (%d) allocation failed", __LINE__);

bail:
	fclose(file);
	return std::make_shared<Image_PNG>(width, height, colorFormat, data);
}

bool Image_PNG::save(const std::string& path) const
{
	bool flag = false;
	FILE* file = fopen(path.c_str(), "wb");
	
	do
	{
		if(!file)
		{
			slog.w(TAG, "can't open file %s for writing", path.c_str());
			break;
		}

/*
	Create and initialize the png_struct with the desired error handler functions.
	If you want to use the default stderr and longjump method,
	you can supply NULL for the last three parameters.  We also check that
	the library version is compatible with the one used at compile time,
	in case we are using dynamically linked libraries.  REQUIRED.
*/
		png_voidp error_ptr = nullptr;
		png_error_ptr error_fn = nullptr;
		png_error_ptr warn_fn = nullptr;
		png_struct* png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, error_ptr, error_fn, warn_fn);
		if(!png_ptr)
		{
			slog.e(TAG, "png_create_write_struct returned 0.");
			break;
		}

		// Allocate/initialize the image information data.  REQUIRED.
		png_info* info_ptr = png_create_info_struct(png_ptr);
		if(!info_ptr || setjmp(png_jmpbuf(png_ptr)))
		{
			slog.e(TAG, "png_create_read_struct returned 0 or set error handling failed.");
			png_destroy_write_struct(&png_ptr, (info_ptr == nullptr)? nullptr : &info_ptr);
			break;
		}

		png_init_io(png_ptr, file);
		
		int color_type;
		switch(colorFormat)
		{
		case Color::G_8:
			color_type = PNG_COLOR_TYPE_GRAY;
			break;
/*
		case PNG_COLOR_TYPE_PALETTE:  // Expand paletted colors into true RGB triplets
			png_set_palette_to_rgb(png_ptr);
			colorFormat = Color::G_8;  //FIXME does palette have alpha?
			break;
*/
		case Color::RGB_888:
			color_type = PNG_COLOR_TYPE_RGB;
			break;

		case Color::GA_88:
			color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
			break;
		default:
			slog.d(TAG, "unknown color format: %d", colorFormat);
			assert(false);
			[[fallthrough]];
		case Color::RGBA_8888:
			color_type = PNG_COLOR_TYPE_RGBA;
			break;
		}

		/* Set the image information here. Width and height are up to 2^31,
		 * bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
		 * the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
		 * PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
		 * or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
		 * PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
		 * currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
		 */
		const int bit_depth = 8;
		png_set_IHDR(png_ptr, info_ptr, this->width, this->height, bit_depth, color_type,
				PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
/*
		// Optional significant bit (sBIT) chunk
		png_color_8 sig_bit;  //FIXME all the color format
		sig_bit.red = bit_depth;
		sig_bit.green = bit_depth;
		sig_bit.blue = bit_depth;
		sig_bit.alpha = bit_depth; // no alpha channel
		png_set_sBIT(png_ptr, info_ptr, &sig_bit);
*/
		// Optionally write comments into the image
		png_text text;
		text.key = const_cast<char*>("Description");  // cast to char*, or -Wno-write-strings to silent warning: deprecated
		text.text = const_cast<char*>("made in Pea"); // conversion from string constant to ‘png_charp {aka char*}’.
		text.compression = PNG_TEXT_COMPRESSION_NONE;
#ifdef PNG_iTXt_SUPPORTED
		text.itxt_length = 0;
		text.lang = nullptr;
		text.lang_key = nullptr;
#endif
		png_set_text(png_ptr, info_ptr, &text, 1);

		// Write the file header information.
		png_write_info(png_ptr, info_ptr);

		if((png_uint_32)this->height > PNG_UINT_32_MAX / (sizeof(png_byte*)))
		{
			slog.e(TAG, "Image is too tall to process in memory");
			break;
		}
		
		std::vector<png_byte*> row_pointers(height);
		uint32_t row_stride = width * Color::size(colorFormat);
		uint8_t* row_pointer = this->data;
//		slog.i(TAG, "width=%d, height=%d, row_stride=%d", width, height, row_stride);
		for(uint32_t k = 0; k < height; ++k)
		{
			row_pointers[k] = row_pointer;
			row_pointer += row_stride;
		}
		
		png_write_image(png_ptr, row_pointers.data());
		png_write_end(png_ptr, info_ptr);

		png_destroy_write_struct(&png_ptr, &info_ptr);
		flag = true;
	} while(false);
	
	fclose(file);
	return flag;
}
