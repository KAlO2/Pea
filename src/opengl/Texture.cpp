#include "opengl/Texture.h"

#include <cinttypes>
#include <cstring>

#include "graphics/ImageFactory.h"
#include "opengl/GL.h"
#include "opengl/Shader.h"
#include "pea/config.h"
#include "util/compiler.h"
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
{
	glGenTextures(1, &name);
}

Texture::~Texture()
{
	glDeleteTextures(1, &name);  // silently ignore name 0
//	name = 0;
}

Texture::Texture(Texture&& other):
		target(other.target),
		name(other.name),
		type(other.type)
{
	other.name = 0;
}

Texture& Texture::operator =(Texture&& other)
{
	if(this != &other)  // ALWAYS check for self-assignment.
	{
		target = other.target;
		
		glDeleteTextures(1, &name);
		name = other.name;
		other.name = 0;
		
		type = other.type;
	}
	return *this;
}

uint32_t Texture::getKey(Texture::Type type, uint32_t index)
{
//	static_cast(std::is_same<std::underlying_type<Texture::Type>, uint8_t>::value);
	constexpr uint32_t MASK = 0x00FFFFFF;
	assert((index & ~MASK) == 0);  // key is made of type (1 byte) and index(3 bytes).
	if((index & ~MASK) != 0)
		slog.w(TAG, "index %" PRIu32 "is out of range [0, 2^24), wrap around", index);
	
	uint32_t key = underlying_cast<Texture::Type>(type) << 24 | (index & MASK);
	return key;
}

uint32_t Texture::getKey(uint32_t index) const
{
	return getKey(type, index);
}

bool Texture::load(const std::string& path)
{
	std::shared_ptr<Image> image = ImageFactory::decodeFile(path);
	if(!image)
	{
		slog.e(TAG, "failed to load image (%s)", path.c_str());
		return false;
	}
	image->flipVertical();
//	assert(image->getColorFormat() == Color::RGBA_8888);
	return load(*image);
}

static int32_t calculateAlignment(int32_t width, Color::Format colorFormat)
{
	int32_t alignment = 1;
	for(int32_t rowStride = width * Color::size(colorFormat); (rowStride & 1) == 0; rowStride >>= 1)
	{
		alignment <<= 1;
		
		// The allowable alignment values are 1, 2, 4, 8
		if(alignment >= 8)
			break;
	}
	
	return alignment;
}

bool Texture::load(const Image& image)
{
//	is_compressed = false;  // TODO: ?
//	this->image = image;
	if(!image.isValid())
		return false;
	
	Color::Format colorFormat = image.getColorFormat();
	GLenum format = GL::pixelFormat(colorFormat);
	GLenum type   = GL::dataType(colorFormat);
	
	GLsizei width  = image.getWidth();
	GLsizei height = image.getHeight();
	if(width == 1 || height == 1)
		assert(target == GL_TEXTURE_1D);
	else
		assert(target == GL_TEXTURE_2D || target == GL_TEXTURE_RECTANGLE);
//	assert(isPowerOfTwo(width) && isPowerOfTwo(height));  // TODO: GL_TEXTURE_RECTANGLE,
	
	int32_t alignment = calculateAlignment(width, colorFormat);
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
	if(target == GL_TEXTURE_2D || target == GL_TEXTURE_RECTANGLE)
		glTexImage2D(target, level, format, width, height, border, format, type, pixels);
	else if(target == GL_TEXTURE_1D)
		glTexImage1D(target, level, format, std::max(width, height), border, format, type, pixels);
//	else if(target == GL_TEXTURE_3D)
//		glTexImage3D(target, level, format, width, height, depth, border, format, type, pixels);
	else
		assert(false);

	return true;
}

bool Texture::loadLevel(uint32_t levelCount, int32_t width, int32_t height, Color::Format colorFormat, const void* data)
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
	
	return true;
}

bool Texture::load(int32_t width, int32_t height, int32_t depth, Color::Format format, const void* data)
{
	assert(target == GL_TEXTURE_3D);
	assert(width > 0 && height > 0 && depth > 0 && data != nullptr);
	
	int32_t alignment = calculateAlignment(width, format);
	glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
	constexpr int32_t level = 0, border = 0;
	
	GLenum pixelFormat = GL::pixelFormat(format);
	GLenum type = GL::dataType(format);
	glTexImage3D(GL_TEXTURE_3D, level, format, width, height, depth, border, pixelFormat, type, data);
	return true;
}

void Texture::setParameter(const Parameter& parameter)
{
	setParameter(target, parameter);
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
bool Texture::loadCube(const std::string paths[6])
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
	
	return loadCube(_images);
}

bool Texture::loadCube(const std::string& directory, const std::string filenames[6])
{
	std::string paths[6];
	for(uint8_t i = 0; i < 6; ++i)
		paths[i] = directory + '/' + filenames[i];
	return loadCube(paths);
}

bool Texture::loadCube(const Image* images[6])
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
	
	return true;
}

static const uint8_t KTX_MAGIC[] =
{
	0xAB, 'k', 'T', 'X', ' ', '1', '1', 0xBB, '\r', '\n', 0x1A, '\n'
};

struct KTX_Header
{
	uint8_t  identifier[12];
	uint32_t endianness;
	uint32_t glType;
	uint32_t glTypeSize;
	uint32_t glFormat;
	uint32_t glInternalFormat;
	uint32_t glBaseInternalFormat;
	uint32_t pixelWidth;
	uint32_t pixelHeight;
	uint32_t pixelDepth;
	uint32_t numberOfArrayElements;
	uint32_t numberOfFaces;
	uint32_t numberOfMipmapLevels;
	uint32_t bytesOfKeyValueData;
};

static void swap32(uint32_t& qword)
{
	uint8_t* bytes = reinterpret_cast<uint8_t*>(&qword);
	std::swap(bytes[0], bytes[3]);
	std::swap(bytes[1], bytes[2]);
}

static uint32_t align(uint32_t value, uint32_t padding = 4)
{
	uint32_t mask = padding - 1;
	return (value + mask) & ~mask;
}
	
bool Texture::loadKTX(const std::string& path)
{
	KTX_Header header;
	
	std::unique_ptr<FILE, decltype(&std::fclose)> file(std::fopen(path.c_str(), "rb"), &std::fclose);
	if(!file)
	{
		slog.w(TAG, "can't open file %s for writing", path.c_str());
		return false;
	}
	
	FILE* fp = file.get();
	if(std::fread(&header, sizeof(header), 1, fp) != 1)
		return false;
	
	if(std::memcmp(header.identifier, KTX_MAGIC, sizeof(KTX_MAGIC)) != 0)
		return false;
	
	constexpr uint32_t KTX_LITTLE_ENDIAN = 0x04030201;
	constexpr uint32_t KTX_BIG_ENDIAN    = 0x01020304;
	if(header.endianness == KTX_LITTLE_ENDIAN)
	{
	}
	else if(header.endianness == KTX_BIG_ENDIAN)
	{
		swap32(header.endianness);
		swap32(header.glType);
		swap32(header.glTypeSize);
		swap32(header.glFormat);
		swap32(header.glInternalFormat);
		swap32(header.glBaseInternalFormat);
		swap32(header.pixelWidth);
		swap32(header.pixelHeight);
		swap32(header.pixelDepth);
		swap32(header.numberOfArrayElements);
		swap32(header.numberOfFaces);
		swap32(header.numberOfMipmapLevels);
		swap32(header.bytesOfKeyValueData);
	}
	else
	{
		slog.e(TAG, "unknown endianness %08X", header.endianness);
		return false;
	}
	
	GLenum target = GL_NONE;
	if(header.pixelHeight == 0)
	{
		if(header.numberOfArrayElements == 0)
			target = GL_TEXTURE_1D;
		else
			target = GL_TEXTURE_1D_ARRAY;
	}
	else if(header.pixelDepth == 0)
	{
		if(header.numberOfArrayElements == 0)
		{
			if(header.numberOfFaces == 0)
				target = GL_TEXTURE_2D;
			else
				target = GL_TEXTURE_CUBE_MAP;
		}
		else
		{
			if(header.numberOfFaces == 0)
				target = GL_TEXTURE_2D_ARRAY;
			else
				target = GL_TEXTURE_CUBE_MAP_ARRAY;
		}
	}
	else
		target = GL_TEXTURE_3D;
	
	if(target == GL_NONE ||
			header.pixelWidth == 0 ||  // Texture has no width?
			(header.pixelHeight == 0 && header.pixelDepth != 0))  // Texture has depth but no height?
		return false;
	
	glBindTexture(target, name);

	size_t dataStart = std::ftell(fp) + header.bytesOfKeyValueData;
	std::fseek(fp, 0, SEEK_END);
	size_t dataEnd = std::ftell(fp);
	fseek(fp, dataStart, SEEK_SET);

	const size_t length = dataEnd - dataStart;
	std::vector<uint8_t> memory(length);
	std::fread(memory.data(), 1, length, fp);
	const uint8_t* data = memory.data();
	
	if(header.numberOfMipmapLevels == 0)
		header.numberOfMipmapLevels = 1;

	switch (target)
	{
	case GL_TEXTURE_1D:
		glTexStorage1D(GL_TEXTURE_1D, header.numberOfMipmapLevels, header.glInternalFormat, header.pixelWidth);
		glTexSubImage1D(GL_TEXTURE_1D, 0, 0, header.pixelWidth, header.glFormat, header.glInternalFormat, data);
		break;
	
	case GL_TEXTURE_2D:
		if(header.glType == GL_NONE)
			glCompressedTexImage2D(GL_TEXTURE_2D, 0, header.glInternalFormat, header.pixelWidth, header.pixelHeight, 0, 420 * 380 / 2, data);
		else
		{
			glTexStorage2D(GL_TEXTURE_2D, header.numberOfMipmapLevels, header.glInternalFormat, header.pixelWidth, header.pixelHeight);
			
			uint32_t channelCount = GL::sizeofChannel(header.glInternalFormat);
			uint32_t height = header.pixelHeight;
			uint32_t width = header.pixelWidth;
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			for(uint32_t i = 0; i < header.numberOfMipmapLevels; ++i)
			{
				glTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, width, height, header.glFormat, header.glType, data);
				data += height * header.glTypeSize * channelCount * width;
				if(height > 1)
					height >>= 1;
				if(width > 1)
					width >>= 1;
			}
			
		}
		break;

	case GL_TEXTURE_3D:
		glTexStorage3D(GL_TEXTURE_3D, header.numberOfMipmapLevels, header.glInternalFormat, header.pixelWidth, header.pixelHeight, header.pixelDepth);
		glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, header.pixelWidth, header.pixelHeight, header.pixelDepth, header.glFormat, header.glType, data);
		break;
	
	case GL_TEXTURE_1D_ARRAY:
		glTexStorage2D(GL_TEXTURE_1D_ARRAY, header.numberOfMipmapLevels, header.glInternalFormat, header.pixelWidth, header.numberOfArrayElements);
		glTexSubImage2D(GL_TEXTURE_1D_ARRAY, 0, 0, 0, header.pixelWidth, header.numberOfArrayElements, header.glFormat, header.glType, data);
		break;
	
	case GL_TEXTURE_2D_ARRAY:
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, header.numberOfMipmapLevels, header.glInternalFormat, header.pixelWidth, header.pixelHeight, header.numberOfArrayElements);
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, header.pixelWidth, header.pixelHeight, header.numberOfArrayElements, header.glFormat, header.glType, data);
		break;

	case GL_TEXTURE_CUBE_MAP:
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, header.numberOfMipmapLevels, header.glInternalFormat, header.pixelWidth, header.pixelHeight);
		{
			uint32_t channelCount = GL::sizeofChannel(header.glInternalFormat);
			uint32_t faceSize = align(header.glTypeSize * channelCount * header.pixelWidth, 4) * header.pixelHeight;
			for(uint32_t i = 0; i < header.numberOfFaces; ++i)
				glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, header.pixelWidth, header.pixelHeight, header.glFormat, header.glType, data + faceSize * i);
	
		}
		break;
	
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, header.numberOfMipmapLevels, header.glInternalFormat, header.pixelWidth, header.pixelHeight, header.numberOfArrayElements);
		glTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, 0, 0, 0, header.pixelWidth, header.pixelHeight, header.numberOfFaces * header.numberOfArrayElements, header.glFormat, header.glType, data);
	break;
	
	default:
		assert(false);  // should never happen.
		return false;
	}
	
	if(header.numberOfMipmapLevels == 1)
		glGenerateMipmap(target);

	return true;
}

void Texture::bind() const
{
	glBindTexture(target, name);
}

void Texture::bind(uint32_t textureUnit) const
{
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(target, name);
}
/*
void Texture::bind(uint32_t textureUnit) const
{
	int32_t location = Shader::UNIFORM_TEX_TEXTURE0 + textureUnit;
	glUniform1i(location, textureUnit);
	bind(textureUnit);
}
*/
// glTexParameteri affects only the currently bound texture.
// it does not affect the way the texture is uploaded (glTexImage2D)
void Texture::setParameter(uint32_t target, const Parameter& parameter)
{
	assert(parameter.levels > 0);
	// Rectangle textures contain exactly one image; they cannot have mipmaps.
	if(parameter.levels > 1 && target != GL_TEXTURE_RECTANGLE)
	{
		// Why OpenGL use two naming conventions: glGen*, glGenerate*?
//		glGenerateTextureMipmap(name);  // OpenGL 4.5+
		glGenerateMipmap(target);  // OpenGL 3.0+
	}
	
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
	case GL_TEXTURE_RECTANGLE:
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

