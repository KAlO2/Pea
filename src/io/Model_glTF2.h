#ifndef PEA_IO_MODEL_GLTF2_H_
#define PEA_IO_MODEL_GLTF2_H_

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include "math/mat4.h"
#include "math/quaternion.h"
#include "opengl/Primitive.h"
#include "opengl/Texture.h"
#include "util/compiler.h"

#include <nlohmann/json.hpp>

namespace pea {
using json = nlohmann::json;

namespace glTF2 {

enum class ComponentType: uint32_t
{
	BYTE           = 5120,
	UNSIGNED_BYTE  = 5121,
	SHORT          = 5122,
	UNSIGNED_SHORT = 5123,
	UNSIGNED_INT   = 5125,
	FLOAT          = 5126,
};

constexpr uint32_t sizeofComponentType(ComponentType type)
{
	switch(type)
	{
	case ComponentType::SHORT:
	case ComponentType::UNSIGNED_SHORT:
		return 2U;

	case ComponentType::UNSIGNED_INT:
	case ComponentType::FLOAT:
		return 4U;

	case ComponentType::BYTE:
	case ComponentType::UNSIGNED_BYTE:
		return 1U;
	
	default:
		throw std::invalid_argument("GLTF: Unsupported Component Type " + 
				std::to_string(pea::underlying_cast(type)));
	}
}

// Keep in sync with ATTRIBTE_TYPES in Model_glTF2.cpp file
enum class AttributeType: uint32_t
{
	SCALAR,
	VEC2,
	VEC3,
	VEC4,
	MAT2,
	MAT3,
	MAT4,
};

std::string attributeTypeToString(AttributeType type);
AttributeType stringToAttributeType(const std::string& text);

enum class MimeType: uint32_t
{
	UNKNOWN,
	BMP,
	JPG,
	PNG,
};

extern const std::string MIME_TYPE_BMP;
extern const std::string MIME_TYPE_JPG;
extern const std::string MIME_TYPE_PNG;

std::string mimeTypeToString(MimeType type);
MimeType stringToMimeType(const std::string& text);

/**
 * alpha mode specifies the interpretation of the alpha value of the main factor and texture.
 */
enum class AlphaMode: uint32_t
{
	/**
	 * The alpha value is ignored and the rendered output is fully opaque.
	 */
	OPAQUE,
	
	/**
	 * The rendered output is either fully opaque or fully transparent depending on the alpha value 
	 * and the specified alpha cutoff value.
	 */
	MASK,
	
	/**
	 * The alpha value is used to composite the source and destination areas. The rendered output is
	 * combined with the background using the normal painting operation (i.e. the Porter and Duff 
	 * over operator).
	 */
	BLEND,
};

constexpr vec4f defaultBaseColor(1, 1, 1, 1);
constexpr vec3f defaultEmissiveFactor(0, 0, 0);
constexpr vec4f defaultDiffuseFactor(1, 1, 1, 1);
constexpr vec3f defaultSpecularFactor(1, 1, 1);
constexpr vec3f defaultSheenFactor(0, 0, 0);
constexpr float defaultAlphaCutoff = 0.5F;
constexpr float defaultMetallicFactor = 1.0F;
constexpr float defaultRoughnessFactor = 1.0F;

class Asset
{
public:
	std::string version;  ///< The glTF version that this asset targets. required.
	std::string generator;  ///< Tool that generated this glTF model. Useful for debugging.
	std::string copyright;  ///< A message suitable for display to credit the content creator.
	std::string minVersion;  ///< The minimum glTF version that this asset targets.
};

/**
 * A buffer is data stored as a binary blob. The buffer can contain a combination of geometry, 
 * animation, and skins.
 *
 */
class Buffer
{
public:
	std::string name;
	std::string uri;
	uint32_t length;
};

struct BufferView
{
public:
	static constexpr uint32_t ARRAY_BUFFER = 34962;
	static constexpr uint32_t ELEMENT_ARRAY_BUFFER = 34963;
	
public:
	std::string name; ///< The user-defined name of this object. Not necessarily unique.
	uint32_t buffer;
	uint32_t offset;
	uint32_t length;  ///< The length of the bufferView in bytes. >= 1
	uint32_t stride;  ///< The stride in bytes, range [4, 252], 0 for tightly packed data.
	uint32_t target;  ///< ARRAY_BUFFER or ELEMENT_ARRAY_BUFFER
};

/**
 * Images referred to by textures are stored in the images array of the asset.
 * Each image contains one of
 * * a URI to an external file in one of the supported images formats, or
 * * a URI with embedded base64-encoded data, or
 * * a reference to a bufferView; in that case mimeType must be defined.
 */
struct Image
{
	std::string name;
	std::string uri;
	
	MimeType mimeType;
	uint32_t bufferView;
	
	std::vector<uint8_t> data;
};

using Sampler = pea::Texture::Parameter;

struct Texture
{
	std::string name;
	int32_t source;
	int32_t sampler;
};

struct TextureInfo
{
	int32_t index;
	int32_t texCoord;  ///< Value 0 corresponds to TEXCOORD_0.
};

struct NormalTextureInfo: TextureInfo
{
	float scale;
};

struct OcclusionTextureInfo: TextureInfo
{
	float strength;
};

struct PbrMetallicRoughness
{
	vec4f baseColorFactor;
	TextureInfo baseColorTexture;
	TextureInfo metallicRoughnessTexture;
	float metallicFactor;
	float roughnessFactor;
public:
	PbrMetallicRoughness():
			baseColorFactor(defaultBaseColor),
			metallicFactor(1.0F),
			roughnessFactor(1.0F)
	{
	}
};

struct PbrSpecularGlossiness
{
	Texture diffuseTexture;
	Texture specularGlossinessTexture;
	vec4f diffuseFactor;
	vec3f specularFactor;
	float glossinessFactor;
};

struct Material
{
	std::string name;
	PbrMetallicRoughness pbrMetallicRoughness;
	
	NormalTextureInfo normalTexture;
	OcclusionTextureInfo occlusionTexture;
	TextureInfo emissiveTexture;
	
	vec3f emissiveFactor;
	
	AlphaMode alphaMode;
	float alphaCutoff;  // cutoff threshold in AlphaMode::MASK mode
	/**
	 * When this value is false, back-face culling is enabled. When this value is true, back-face 
	 * culling is disabled and double sided lighting is enabled. The back-face must have its normals
	 * reversed before the lighting equation is evaluated.
	 */
	bool doubleSided;
public:
	Material():
			emissiveFactor(defaultEmissiveFactor),
			alphaCutoff(defaultAlphaCutoff),
			doubleSided(false)
	{
	}
};

/**
 * A node can have either a matrix or any combination of translation/rotation/scale (TRS)
 * properties. When a node is targeted for animation (referenced by an animation.channel.target),
 * only TRS properties may be present; matrix will not be present.
 */
struct Node
{
	std::string name;
	
	std::vector<uint32_t> children;
	int32_t mesh;
	int32_t camera;
	
	mat4f matrix;
	
	vec3f translation;
	quaternionf rotation;
	vec3f scale;
	bool hasMatrix;
	
	std::vector<float> weights;
public:
	void resetTransform();
	mat4f getTransform() const;
};

struct Scene
{
	std::string name;
	std::vector<uint32_t> nodes;
};

/**
 * All large data for meshes, skins, and animations is stored in buffers and retrieved via accessors.
 */
struct Accessor
{
	std::string name;
	uint32_t bufferView;
	uint32_t byteOffset;
	
	ComponentType componentType;
	
	/**
	 * Specifies whether integer data values should be normalized (true) to [0, 1] (for unsigned 
	 * types) or [-1, 1] (for signed types), or converted directly (false) when they are accessed.
	 */
	bool normalized;
	
	uint32_t count;  // The number of attributes referenced by this accessor, >= 1
	AttributeType type;  // Specifies if the attribute is a scalar, vector, or matrix.
	
	std::vector<float> min, max;
};

struct Primitive
{
	/**
	 * (required) A dictionary object of integer, where each integer
	 * is the index of the accessor containing an attribute.
	 * position, normal, tangent, texcoord, color, joint, jointmatrix, weight
	 */
	std::unordered_map<std::string, int32_t> attributes;
	int32_t indices;
	int32_t material;
	pea::Primitive mode;
};

struct Mesh
{
	std::string name;
	std::vector<Primitive> primitives;
	std::vector<float> weights;
};

struct Camera
{
	std::string name;
	bool perspective;
	union
	{
		struct {float aspectRatio, yfov;};
		struct {float xmag, ymag;};  // horizontal/vertical magnification of the view
	};
	float znear, zfar;
};

/**
 * @see https://github.com/KhronosGroup/glTF/blob/master/specification/2.0/README.md
 */
struct glTF
{
	Asset asset;
	std::vector<Buffer> buffers;
	std::vector<BufferView> bufferViews;
	std::vector<Accessor> accessors;
	std::vector<Camera> cameras;
	std::vector<Scene> scenes;
	std::vector<Image> images;
	std::vector<Texture> textures;
	std::vector<Material> materials;
	std::vector<Mesh> meshes;
	std::vector<Node> nodes;
	int32_t scene;  // The index of the default scene.
};

void to_json(json& j, const Asset& asset);
void from_json(const json& j, Asset& asset);

void to_json(json& j, const Buffer& buffer);
void from_json(const json& j, Buffer& buffer);

void to_json(json& j, const BufferView& bufferView);
void from_json(const json& j, BufferView& bufferView);

void to_json(json& j, const Accessor& accessor);
void from_json(const json& j, Accessor& accessor);

void to_json(json& j, const Camera& camera);
void from_json(const json& j, Camera& camera);

void to_json(json& j, const Image& image);
void from_json(const json& j, Image& image);

void to_json(json& j, const Texture& texture);
void from_json(const json& j, Texture& texture);

void to_json(json& j, const TextureInfo& textureInfo);
void from_json(const json& j, TextureInfo& textureInfo);

void to_json(json& j, const NormalTextureInfo& textureInfo);
void from_json(const json& j, NormalTextureInfo& textureInfo);

void to_json(json& j, const OcclusionTextureInfo& textureInfo);
void from_json(const json& j, OcclusionTextureInfo& textureInfo);

void to_json(json& j, const Material& material);
void from_json(const json& j, Material& material);

void to_json(json& j, const PbrMetallicRoughness& pbr);
void from_json(const json& j, PbrMetallicRoughness& pbr);

void to_json(json& j, const Primitive& primitive);
void from_json(const json& j, Primitive& primitive);

void to_json(json& j, const Mesh& mesh);
void from_json(const json& j, Mesh& mesh);

void to_json(json& j, const Node& node);
void from_json(const json& j, Node& node);

void to_json(json& j, const Scene& scene);
void from_json(const json& j, Scene& scene);

void to_json(json& j, const Camera& camera);
void from_json(const json& j, Camera& camera);

void to_json(json& j, const glTF& model);
void from_json(const json& j, glTF& model);

}  // namespace glTF2

class Model_glTF2
{
public:
	glTF2::glTF model;
	
public:
	explicit Model_glTF2(const std::string& path) noexcept(false);
	
	bool save(const std::string& path, uint32_t space = 4) const;
	
};

}  // namespace pea
#endif  // PEA_IO_MODEL_GLTF2_H_
