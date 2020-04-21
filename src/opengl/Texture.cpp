#include "opengl/Texture.h"

#include "graphics/ImageFactory.h"
#include "opengl/GL.h"
#include "opengl/Shader.h"
#include "pea/config.h"
#include "util/Log.h"


using namespace pea;

static const char* TAG = "Texture";

const int32_t Texture::MAX_SIZE = 4096;

int32_t Texture::nextPowerOfTwo(int32_t n)
{
	if(isPowerOfTwo(n))
		return n;

	--n;
	n = n | (n >> 1);
	n = n | (n >> 2);
	n = n | (n >> 4);
	n = n | (n >> 8);
	n = n | (n >>16);
	return n + 1;
}

Texture::Texture():
		Texture(GL_TEXTURE_2D)
{
}

Texture::Texture(uint32_t target):
		target(target),
		name(0),
		type(Type::NONE)
//		image(nullptr),
//		mipmapImage(nullptr)
//		has_mipmap(false),
//		is_compressed(false)
{
	glGenTextures(1, &name);
}

Texture::~Texture()
{
//	slog.d(TAG, "free Texture2D resource %u", name);
	glDeleteTextures(1, &name);  // silently ignore name 0
//	texture_id = 0;
//	delete image;
//	delete[] mipmapImage;
	//FIXME cast to right type, then delete.
	// raw data cannot be delete'd.
//	delete[] texture_data;
//	texture_data = nullptr;
}

Texture::Texture(Texture&& other):
		target(other.target),
		name(other.name)
{
	other.name = 0;
}

Texture& Texture::operator =(Texture&& other)
{
	if(this != &other)  // ALWAYS check for self-assignment.
	{
		glDeleteTextures(1, &name);
		target = other.target;
		name = other.name;
		other.name = 0;
	}
	return *this;
}

bool Texture::load(const std::string& path, const Parameter& parameter)
{
	std::shared_ptr<Image> image = ImageFactory::decodeFile(path);
	if(!image)
	{
		slog.e(TAG, "failed to load image (%s)", path.c_str());
		return false;
	}
	image->flipVertical();
//	assert(image->getColorFormat() == Color::RGBA_8888);
	return load(*image, parameter);
}

//bool load(const std::string paths[6], const Parameter& parameter);
bool Texture::load(const Image& image, const Parameter& parameter)
{
	is_compressed = false;  // TODO: ?
//	this->image = image;
	if(!image.isValid())
		return false;
	
	Color::Format colorFormat = image.getColorFormat();
	GLenum format = GL::pixelFormat(colorFormat);
	GLenum type   = GL::dataType(colorFormat);
	
	GLsizei width  = image.getWidth();
	GLsizei height = image.getHeight();
	GLenum target = (width == 1 || height == 1) ? GL_TEXTURE_1D: GL_TEXTURE_2D;
	assert(this->target == target);
//	assert(isPowerOfTwo(width) && isPowerOfTwo(height));  // TODO: GL_TEXTURE_RECTANGLE,
	
	int32_t alignment = 1;
	for(int32_t rowStride = width * Color::size(colorFormat); (rowStride & 1) == 0; rowStride >>= 1)
	{
		alignment <<= 1;
		
		// The allowable alignment values are 1, 2, 4, 8
		if(alignment >= 8)
			break;
	}
	slog.v(TAG, "width=%d, pixel size=%d, alignment=%d", width, Color::size(colorFormat), alignment);
	// Pixel data in user memory is said to be packed. Therefore, transfers to OpenGL memory are 
	// called unpack operations, and transfers from OpenGL memory are called pack operations.
	glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);//alignment);  // affect the operation of subsequent glTexImage
	
	glBindTexture(target, name);
	constexpr GLint level = 0;  // level-of-detail number. Level 0 is the base image level.
	constexpr GLint border = 0;  // Specifies the width of the border. Must be either 0 or 1.
	const uint8_t* pixels = image.getData();
	assert(width > 0 && height > 0 && pixels != nullptr);
	// should glTexParameteri come before or after glTexImage2D?
	// https://community.khronos.org/t/gltexparameteri-before-glteximage2d/23056
	if(target == GL_TEXTURE_2D)
		glTexImage2D(target, level, format, width, height, border, format, type, pixels);
	else if(target == GL_TEXTURE_1D)
		glTexImage1D(target, level, format, std::max(width, height), border, format, type, pixels);
//	else if(target == GL_TEXTURE_3D)
//		glTexImage3D(target, level, format, width, height, depth, border, format, type, pixels);
	else
		assert(false);
	// why two naming convention: glGen*, glGenerate*
	if(parameter.levels > 1)
		glGenerateTextureMipmap(name);  // glGenerateMipmap(target);

	setParameter(target, parameter);
	return true;
}

bool Texture::loadLevel(uint32_t levelCount, uint32_t width, uint32_t height, Color::Format colorFormat, const void* data, const Parameter& parameter)
{
	assert(levelCount > 0 && width > 0 && height > 0 && data != nullptr);
	GLenum format = GL::pixelFormat(colorFormat);
	GLenum type   = GL::dataType(colorFormat);
	uint32_t pixelSize = Color::size(colorFormat);
	const uint8_t* _data = reinterpret_cast<const uint8_t*>(data);
	
	glBindTexture(target, name);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // always 1 byte aligned
	constexpr GLint border = 0;  // Specifies the width of the border. Must be either 0 or 1.
	for(uint32_t level = 0; level < levelCount; ++level)
	{
		if(target == GL_TEXTURE_2D)
			glTexImage2D(target, level, format, width, height, border, format, type, _data);
		else if(target == GL_TEXTURE_1D)
			glTexImage1D(target, level, format, std::max(width, height), border, format, type, _data);
		
		_data += width * height * pixelSize;
		
		if(width != 1 || height != 1)
		{
			if(width != 1)
				width >>= 1;
			
			if(height != 1)
				height >>= 1;
		}
		else  // width == 1 && height == 1
			break;
	}
	
	setParameter(target, parameter);
	return true;
}

static bool isSame(const std::string& path1, const std::string& path2)
{
	size_t length1 = path1.length();
	size_t length2 = path2.length();
	if(length1 != length2)
		return false;
	
	for(size_t i = length1; i-- > 0;)
		if(path1[i] != path2[i])
			return false;
	
	return true;
}

/*
https://www.khronos.org/opengl/wiki/Cubemap_Texture
       +------+
       |  +Y  |
       | top  |
+------+------+------+------+
|  -X  |  +Z  |  +X  |  -Z  |
| left |front |right | back |
+------+------+------+------+
       |  -Y  |
       |bottom|
       +------+
*/
bool Texture::loadCube(const std::string paths[6], const Parameter& parameter)
{
	std::shared_ptr<Image> images[6];
	
	for(uint8_t i = 0; i < 6; ++i)
	{
		bool loaded = false;
		for(uint8_t j = 0; j < i; ++j)
		{
			if(isSame(paths[i], paths[j]))
			{
				images[i] = images[j];
				loaded = true;
			}
		}
		if(loaded)
			continue;
		
		images[i] = ImageFactory::decodeFile(paths[i]);
		if(!images[i])
		{
			slog.w(TAG, "loading texture \"%s\" failed", paths[i].c_str());
			return false;
		}
	}
	
	const Image* _images[6];
	for(uint8_t i = 0; i < 6; ++i)
		_images[i] = images[i].get();
	
	return loadCube(_images, parameter);
}

bool Texture::loadCube(const Image* images[6], const Parameter& parameter)
{
	constexpr GLenum target = GL_TEXTURE_CUBE_MAP;
	assert(this->target == target);
	constexpr GLint level = 0;  // level-of-detail number. Level 0 is the base image level.

	constexpr GLint border = 0;  // Specifies the width of the border. Must be either 0 or 1.
	glBindTexture(target, name);
	
	constexpr GLenum targets[6] =
	{
		// OpenGL macro order: +X, -X, +Y, -Y, +Z, -Z.
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
#if USE_MATH_XYZ
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
#else
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
#endif
	};
	
	for(int8_t i = 0; i < 6; ++i)
	{
		const Image* image = images[i];
		assert(image);
		Color::Format colorFormat = image->getColorFormat();
		const uint8_t* pixels = image->getData();
		
		GLsizei width  = image->getWidth();
		GLsizei height = image->getHeight();
		GLenum format = GL::pixelFormat(colorFormat);
		GLenum type   = GL::dataType(colorFormat);
		glTexImage2D(targets[i], level, format, width, height, border, format, type, pixels);
	}
	if(parameter.levels > 1)
		glGenerateTextureMipmap(name);
	setParameter(target, parameter);
	
	return true;
}

void Texture::bind() const
{
	glBindTexture(target, name);
}

void Texture::bind(uint32_t textureUnit) const
{
	bind(Shader::UNIFORM_TEX_TEXTURE0 + textureUnit, textureUnit);
}

void Texture::bind(uint32_t location, uint32_t textureUnit) const
{
	glUniform1i(location, textureUnit);
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(target, name);
}

void Texture::setParameter(uint32_t target, const Parameter& parameter)
{
	assert(parameter.levels > 0);
	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, parameter.levels - 1);

	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, parameter.minFilter);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, parameter.magFilter);
	
	switch(target)
	{
	case GL_TEXTURE_CUBE_MAP:
		glTexParameteri(target, GL_TEXTURE_WRAP_R, parameter.mapW);
		[[fallthrough]];
	case GL_TEXTURE_2D:
		glTexParameteri(target, GL_TEXTURE_WRAP_T, parameter.mapV);
		[[fallthrough]];
	case GL_TEXTURE_1D:
		glTexParameteri(target, GL_TEXTURE_WRAP_S, parameter.mapU);
		break;
	default:
		assert(false);
	}
}

Texture::Parameter::Parameter(uint32_t levels/* = 1*/):
		minFilter(levels > 1? GL_LINEAR_MIPMAP_LINEAR: GL_LINEAR),
		magFilter(GL_LINEAR),
		mapU(GL_CLAMP_TO_EDGE),
		mapV(GL_CLAMP_TO_EDGE),
		mapW(GL_CLAMP_TO_EDGE),
		levels(levels)
{
	assert(levels > 0);
}

Texture::Parameter& Texture::Parameter::operator =(const Texture::Parameter& other)
{
	if(this != &other)
	{
//		std::memcpy(this, &other, sizeof(Texture::Parameter));
		minFilter = other.minFilter;
		magFilter = other.magFilter;
		mapU      = other.mapU;
		mapV      = other.mapV;
		mapW      = other.mapW;
		levels    = other.levels;
	}
	return *this;
}

void Texture::Parameter::setMapMode(uint32_t mode)
{
	mapU = mapV = mapW = mode;
}

