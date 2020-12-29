#ifndef PEA_OPENGL_TEXTURE_H_
#define PEA_OPENGL_TEXTURE_H_

#include <memory>

#include "graphics/Image.h"
#include "io/Type.h"
#include "math/vec2.h"


namespace pea {

/**
 * @brief Wraps OpenGL texture objects.
 * A Texture provides both interfaces to bind them for the OpenGL pipeline:
 * binding and bindless texture. Bindless textures are only available if the
 * graphics driver supports them.
 * Note that Image's origin is top left, while texture's origin is bottom left.
 *
 * @see http://www.opengl.org/wiki/Texture
 * @see http://www.opengl.org/registry/specs/NV/bindless_texture.txt
 * @see https://www.khronos.org/opengl/wiki/Common_Mistakes
 */
class Texture final
{
public:
	static const int32_t MAX_SIZE;

	enum class Filter
	{
		NEAREST,
		LINEAR,
		QUADRATIC,
		CUBIC,
	};

	enum Type: uint8_t
	{
		NONE      = 0,
		AMBIENT   = 1,
		DIFFUSE   = 2,
		SPECULAR  = 3,
		SHININESS = 4,
		EMISSIVE  = 5,
		HEIGHT    = 6,
		NORMAL    = 7,
		DEPTH     = 8,
		BUFFER    = 9,  // Texture Buffer Object
		
		UNKNOWN = 255,
	};
	
	enum Dimension: uint8_t
	{
		_1,  // sampler1D
		_2,  // sampler2D
		_3,  // sampler3D
		CUBE,  // samplerCube
		_1_ARRAY,    // sampler1DArray
		_2_ARRAY,    // sampler2DArray
		CUBE_ARRAY,  // samplerCubeArray
	};

	enum Mapping: uint32_t
	{
		MAPPING_UV,
		MAPPING_PLANE,
		MAPPING_BOX,
		MAPPING_CYLINDER,
		MAPPING_SPHERE,
	};

	/**
	 * @brief Defines how UV coordinates outside the [0, 1] range are handled.
	 */
	enum class MapMode: uint8_t
	{
		/**
		 * wrap (DirectX), repeat (OpenGL), or tile. f(t) = mod(t, 1.0)
		 * the image repeats itself across the surface.
		 */
		WRAP,

		/**
		 * f(t) = 1 - std::abs(t-1) for range [0, 2], f(t+2) = f(t)
		 * The image repeats itself across the surface, but it's mirrored(flipped)
		 * on every other repetition. This provides some continuity along the edge
		 * of the texture.
		 */
		MIRROR,

		/**
		 * clamp (DirectX) or clamp to edge (OpenGL). f(t) = std::clamp(t, 0, 1.0)
		 * Values outside the range [0, 1) are clamped to this range.
		 */
		CLAMP,

		/**
		 * border (DirectX) or clamp to border (OpenGL).
		 * Values outside the range [0, 1) are rendered with a separately defined
		 * border color. This function can be good for rendering decals onto
		 * surface, for example, as the edge of the texutre will blend smoothly
		 * with the border color.
		 */
		DECAL,
	};
	
	class Parameter;

protected:
	uint32_t target;
	
	/**
	 * size of the texture in pixels.
	 * If size.height is zero, the texture is compressed in a format like JPEG.
	 * In this case, size.width specifies the size of the memory in bytes.
	 */
	uint32_t name;  ///< texture name
	Type type;

// vim /home/martin/Source/irrlicht-1.8.4/source/Irrlicht/COpenGLTexture.h
//	bool is_compressed;

public:
//	static Texture generateCheckboard()
	static int32_t nextPowerOfTwo(int32_t n);

	static void setParameter(uint32_t target, const Parameter& parameter);
	
	static uint32_t getKey(Type type, uint32_t index);
	
public:
	explicit Texture(uint32_t target);
	Texture();
	~Texture();

	Texture(const Texture& other) = delete;
	Texture& operator =(const Texture& other) = delete;
	
	Texture(Texture&& other);
	Texture& operator =(Texture&& other);
	
	const uint32_t& getTarget() const;
	
	/**
	 * @return texture name.
	 */
	const uint32_t& getName() const;
	
	void setType(Type type);
	Type getType() const;
	
	uint32_t getKey(uint32_t index) const;
	
	bool load(const std::string& path);
	bool load(const Image& image);
	
	/**
	 * OpenGL uses textures that are exact powers of 2 in width and height. A 64x16 2D texture can 
	 * have 6 mip-maps: 32x8, 16x4, 8x2, 4x1, 2x1, and 1x1. OpenGL does not require that the entire 
	 * mipmap chain is complete; you can specify what range of mipmaps in a texture are available. 
	 */
	bool loadLevel(uint32_t levelCount, int32_t width, int32_t height, Color::Format colorFormat, const void* data);
	
	/**
	 * Used to load 3D textures.
	 */
	bool load(int32_t width, int32_t height, int32_t depth, Color::Format format, const void* data);
	
	void setParameter(const Parameter& parameter);
	
	/**
	 * load texture cube.
	 */
	bool loadCube(const std::string paths[6]);

	/**
	 * @brief Create a skybox from 6 textures.
	 *
	 * @param[in] directory the directory of the texture file.
	 * @param[in] filenames Texture filename for the left, right, back, front, bottom, top side of
	 *                      the texture cube face.
	 * @return true if textures are loaded successfully, otherwise false.
	 */
	bool loadCube(const std::string& directory, const std::string filenames[6]);
	bool loadCube(const Image* images[6]);

	/**
	 * Attach a buffer object's data store to a buffer texture object.
	 * @param[in] type Specifies the format of the data in the store belonging to buffer.
	 * @param[in] buffer Texture buffer created by @link Buffer. If buffer is zero, any buffer
	 *                   object attached to the buffer texture is detached
	 */
	void attachBuffer(pea::Type type, uint32_t buffer);
	
	/**
	 * KTX file format https://www.khronos.org/opengles/sdk/tools/KTX/file_format_spec/
	 */
	bool loadKTX(const std::string& path);
	
	/**
	 * Update texture data. It use gl*TexSubImage() to replace all or part of an existing texture
	 * image, rather than the more costly operations of deleting and creating an entire new image.
	 *
	 * @param offset Specifies texel offset with coordinate (x, y)
	 * @param size The size of subimage to be replaced, (width, height)
	 * @param data Pointer of an image data in the memory
	 */
//	void update(const vec2i& offset, const vec2i& size, const uint8_t* data);
	void bind() const;

	void bind(uint32_t textureUnit) const;

//	bool isCompressed() const { return is_compressed; }
//	bool hasMipmaps()   const { return mipmap != nullptr; }

	
};

inline const uint32_t& Texture::getTarget() const { return target; }
inline const uint32_t& Texture::getName() const { return name; }

inline void Texture::setType(Texture::Type type) { this->type = type; }
inline Texture::Type Texture::getType() const    { return type;       }

class Texture::Parameter
{
public:
	uint32_t minFilter;  ///< OpenGL defaults GL_TEXTURE_MIN_FILTER to GL_NEAREST_MIPMAP_LINEAR
	uint32_t magFilter;
	uint32_t mapU;
	uint32_t mapV;
	uint32_t mapW;
	uint32_t levels;  ///< OpenGL defaults GL_TEXTURE_MAX_LEVEL to be 1000
	
	Parameter(uint32_t levels = 1);
	Parameter(const Parameter& other) = default;
	Parameter& operator =(const Parameter& other);
	
	/**
	 * @param[in] mode GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER, GL_MIRRORED_REPEAT, GL_REPEAT, or
	 *                 GL_MIRROR_CLAMP_TO_EDGE.
	 */
	void setMapMode(uint32_t mode);
};

}  // namespace pea
#endif  // PEA_OPENGL_TEXTURE_H_
