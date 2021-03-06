#include "io/Model_glTF2.h"

#include <fstream>

#include "util/type_trait.h"

namespace pea {
namespace glTF2 {

static const std::string ATTRIBTE_TYPES[] = {"SCALAR", "VEC2", "VEC3", "VEC4", "MAT2", "MAT3", "MAT4"};

std::string attributeTypeToString(AttributeType type)
{
	constexpr uint32_t size = sizeof(ATTRIBTE_TYPES) / sizeof(ATTRIBTE_TYPES[0]);
	auto index = underlying_cast(type);
	if(index >= size)
		throw std::invalid_argument("GLTF: Unsupported Attribute Type " + std::to_string(index));
	return ATTRIBTE_TYPES[index];
}

AttributeType stringToAttributeType(const std::string& text)
{
	constexpr uint32_t size = sizeof(ATTRIBTE_TYPES) / sizeof(ATTRIBTE_TYPES[0]);
	for(uint32_t i = 0; i < size; ++i)
		if(text == ATTRIBTE_TYPES[i])
			return static_cast<AttributeType>(i);
	
	throw std::invalid_argument("GLTF: Unsupported Attribute Type " + text);
}

const std::string MIME_TYPE_BMP = "image/bmp";
const std::string MIME_TYPE_JPG = "image/jpeg";
const std::string MIME_TYPE_PNG = "image/png";

std::string mimeType2String(MimeType type)
{
	switch(type)
	{
	case MimeType::BMP:  return MIME_TYPE_BMP;
	case MimeType::JPG:  return MIME_TYPE_JPG;
	case MimeType::PNG:  return MIME_TYPE_PNG;
	
	default:
		throw std::invalid_argument("GLTF: Unsupported MIME Type " + 
				std::to_string(pea::underlying_cast(type)));
	}
}

MimeType string2MimeType(const std::string& text)
{
	if(text == MIME_TYPE_BMP)
		return MimeType::BMP;
	else if(text == MIME_TYPE_JPG)
		return MimeType::JPG;
	else if(text == MIME_TYPE_PNG)
		return MimeType::PNG;
	else
		return MimeType::UNKNOWN;
}

template <typename T>
static void assign(json& j, const std::string& key, const T& value)
{
	if constexpr(std::is_same<T, int32_t>::value)
	{
		if(value >= 0)
			j[key] = value;
	}
	else if constexpr(std::is_same<T, std::string>::value)
	{
		if(!value.empty())
			j[key] = value;
	}
	else
		static_assert(dependent_false<T>::value, "unsupported type");
}

template <typename T>
static inline void extract(const json& j, const std::string& key, T& value)
{
	if(j.contains(key))
	{
		j[key].get_to(value);
		if constexpr(std::is_same<T, int32_t>::value)
		{
			if(value < 0)
				throw std::out_of_range("negative index" + std::to_string(value));
		}
	}
	else
	{
		if constexpr(std::is_same<T, int32_t>::value)
			value = -1;
		else if constexpr(std::is_same<T, std::string>::value ||
				is_vector<T>::value)
			value.clear();
	}
}

void to_json(json& j, const Asset& asset)
{
	j["version"] = asset.version;
	if(!asset.generator.empty())
		j["generator"]  = asset.generator;
	if(!asset.copyright.empty())
		j["copyright"]  = asset.copyright;
	if(!asset.minVersion.empty())
		j["minVersion"] = asset.minVersion;
}

void from_json(const json& j, Asset& asset)
{
	j.at("version").get_to(asset.version);
	if(j.contains("generator"))
		j.at("generator").get_to(asset.generator);
	if(j.contains("copyright"))
		j.at("copyright").get_to(asset.copyright);
	if(j.contains("minVersion"))
		j.at("minVersion").get_to(asset.minVersion);
}

void to_json(json& j, const Buffer& buffer)
{
	assign(j, "name", buffer.name);
	j["uri"] = buffer.uri;
	j["byteLength"] = buffer.length;
}

void from_json(const json& j, Buffer& buffer)
{
	extract(j, "name", buffer.name);
	j.at("uri").get_to(buffer.uri);
	j.at("byteLength").get_to(buffer.length);
}

void to_json(json& j, const BufferView& bufferView)
{
	assign(j, "name", bufferView.name);
	j["buffer"] = bufferView.buffer;
	if(bufferView.offset > 0)
		j["byteOffset"] = bufferView.offset;
	j["byteLength"] = bufferView.length;
}

void from_json(const json& j, BufferView& bufferView)
{
	extract(j, "name", bufferView.name);
	j.at("buffer").get_to(bufferView.buffer);
	if(j.contains("byteOffset"))
		j.at("byteOffset").get_to(bufferView.offset);
	else
		bufferView.offset = 0;
	j.at("byteLength").get_to(bufferView.length);
}

void to_json(json& j, const Accessor& accessor)
{
	assign(j, "name", accessor.name);
	j["bufferView"] = accessor.bufferView;
	if(accessor.byteOffset > 0)
		j["byteOffset"] = accessor.byteOffset;
	j["componentType"] = underlying_cast(accessor.componentType);
	j["count"] = accessor.count;
	j["type"] = attributeTypeToString(accessor.type);
	if(!accessor.min.empty())
		j["min"] = accessor.min;
	if(!accessor.max.empty())
		j["max"] = accessor.max;
}

void from_json(const json& j, Accessor& accessor)
{
	extract(j, "name", accessor.name);
	j.at("bufferView").get_to(accessor.bufferView);
	if(j.contains("byteOffset"))
		j.at("byteOffset").get_to(accessor.byteOffset);
	else
		accessor.byteOffset = 0;
	{
		uint32_t type;
		j.at("componentType").get_to(type);
		accessor.componentType = static_cast<ComponentType>(type);
		type = sizeofComponentType(accessor.componentType);  // validate componentType
	}
	j.at("count").get_to(accessor.count);
	{
		std::string type;
		j.at("type").get_to(type);
		accessor.type = stringToAttributeType(type);
	}
	if(j.contains("min"))
		j.at("min").get_to(accessor.min);
	if(j.contains("max"))
		j.at("max").get_to(accessor.max);
	
	size_t length = accessor.min.size();
	if(length != accessor.max.size())
		throw std::invalid_argument("min and max's length are not match");
	if(length > 4 && length != 9 && length != 16)
		throw std::invalid_argument("invalid Number of components" + std::to_string(length));
}

void to_json(json& j, const Camera& camera)
{
	assign(j, "name", camera.name);
	if(camera.perspective)
	{
		j["type"] = "perspective";
		auto& perspective = j["perspective"];
		// glPerspective(float fieldOfView, float aspectRatio, float near, float far);
		perspective["yfov"] = camera.yfov;
		perspective["aspectRatio"] = camera.aspectRatio;
		perspective["znear"] = camera.znear;
		perspective["zfar"] = camera.zfar;
	}
	else
	{
		j["type"] = "orthographic";
		auto& orthographic = j["orthographic"];
		orthographic["xmag"] = camera.xmag;
		orthographic["ymag"] = camera.ymag;
		orthographic["znear"] = camera.znear;
		orthographic["zfar"] = camera.zfar;
	}
}

void from_json(const json& j, Camera& camera)
{
	extract(j, "name", camera.name);
	camera.perspective = j.at("type") == "perspective";
	if(camera.perspective)
	{
		const json& perspective = j.at("perspective");
		perspective.at("yfov").get_to(camera.yfov);
		perspective.at("aspectRatio").get_to(camera.aspectRatio);
		perspective.at("znear").get_to(camera.znear);
		auto it = perspective.find("zfar");
		if(it != perspective.end())
			camera.zfar = it->get<float>();
		else
			camera.zfar = std::numeric_limits<float>::infinity();
	}
	else
	{
		const json& orthographic = j.at("orthographic");
		orthographic.at("xmag").get_to(camera.xmag);
		orthographic.at("ymag").get_to(camera.ymag);
		orthographic.at("znear").get_to(camera.znear);
		orthographic.at("zfar").get_to(camera.zfar);
	}
}

void to_json(json& j, const Image& image)
{
	assign(j, "name", image.name);
//	assign(j, "uri", image.uri);
	if(!image.uri.empty())
		j["uri"] = image.uri;
	else
	{
		j["mimeType"] = mimeType2String(image.mimeType);
		j["bufferView"] = image.bufferView;
	}
}

void from_json(const json& j, Image& image)
{
	extract(j, "name", image.name);
	if(j.contains("mimeType"))
	{
		std::string text;
		j.at("mimeType").get_to(text);
		image.mimeType = string2MimeType(text);
	}

	if(j.contains("uri"))
		j.at("uri").get_to(image.uri);
	else
		j.at("bufferView").get_to(image.bufferView);
}

void to_json(json& j, const Texture& texture)
{
	assign(j, "name", texture.name);
	if(texture.source >= 0)
		j["source"] = texture.source;
	if(texture.sampler >= 0)
		j["sampler"] = texture.sampler;
}

void from_json(const json& j, Texture& texture)
{
	extract(j, "name", texture.name);
	if(j.contains("source"))
		j.at("source").get_to(texture.source);
	else
		texture.source = -1;
	if(j.contains("sampler"))
		j.at("sampler").get_to(texture.sampler);
	else
		texture.sampler = -1;
}

void to_json(json& j, const TextureInfo& textureInfo)
{
	j["index"] = textureInfo.index;
	if(textureInfo.texCoord > 0)
		j["texCoord"] = textureInfo.texCoord;
}

void from_json(const json& j, TextureInfo& textureInfo)
{
	j.at("index").get_to(textureInfo.index);
	if(j.contains("texCoord"))
		j.at("texCoord").get_to(textureInfo.texCoord);
	else
		textureInfo.texCoord = 0;
}

void to_json(json& j, const NormalTextureInfo& textureInfo)
{
	to_json(j, static_cast<const TextureInfo&>(textureInfo));
	if(textureInfo.scale != 1)
		j["scale"] = textureInfo.scale;
}

void from_json(const json& j, NormalTextureInfo& textureInfo)
{
	from_json(j, static_cast<TextureInfo&>(textureInfo));
	if(j.contains("scale"))
		j.at("scale").get_to(textureInfo.scale);
	else
		textureInfo.scale = 1;
}

void to_json(json& j, const OcclusionTextureInfo& textureInfo)
{
	to_json(j, static_cast<const TextureInfo&>(textureInfo));
	if(textureInfo.strength != 1)
		j["strength"] = textureInfo.strength;
}

void from_json(const json& j, OcclusionTextureInfo& textureInfo)
{
	from_json(j, static_cast<TextureInfo&>(textureInfo));
	if(j.contains("strength"))
		j.at("strength").get_to(textureInfo.strength);
	else
		textureInfo.strength = 1;
}

static const std::string AlphaModeTexts[3] = {"OPAQUE", "MASK", "BLEND"};
constexpr AlphaMode defaultAlphaMode = AlphaMode::OPAQUE;

void to_json(json& j, const Material& material)
{
	assign(j, "name", material.name);
	to_json(j["pbrMetallicRoughness"], material.pbrMetallicRoughness);
	if(material.normalTexture.index >= 0)
		to_json(j["normalTexture"], material.normalTexture);
	if(material.occlusionTexture.index >= 0)
		to_json(j["occlusionTexture"], material.occlusionTexture);
	if(material.alphaMode != defaultAlphaMode)
		j["alphaMode"] = AlphaModeTexts[underlying_cast(material.alphaMode)];
	if(material.alphaCutoff != defaultAlphaCutoff)
		j["alphaCutoff"] = material.alphaCutoff;
	if(material.doubleSided)
		j["doubleSided"] = material.doubleSided;
}

void from_json(const json& j, Material& material)
{
	extract(j, "name", material.name);
	
	if(j.contains("pbrMetallicRoughness"))
		from_json(j["pbrMetallicRoughness"], material.pbrMetallicRoughness);
	
	if(j.contains("normalTexture"))
		from_json(j["normalTexture"], material.normalTexture);
	else
		material.normalTexture.index = -1;
	
	if(j.contains("occlusionTexture"))
		from_json(j["occlusionTexture"], material.occlusionTexture);
	else
		material.occlusionTexture.index = -1;
	
	material.alphaMode = AlphaMode::OPAQUE;
	if(j.contains("alphaMode"))
	{
		std::string text;
		j.at("alphaMode").get_to(text);
		
		constexpr uint32_t size = sizeof(AlphaModeTexts) / sizeof(AlphaModeTexts[0]);
		for(uint32_t i = 0; i < size; ++i)
		{
			if(text == AlphaModeTexts[i])
			{
				material.alphaMode = static_cast<AlphaMode>(i);
				break;
			}
		}
	}
	
	if(j.contains("alphaCutoff"))
		j.at("alphaCutoff").get_to(material.alphaCutoff);
	else
		material.alphaCutoff = defaultAlphaCutoff;
	
	if(j.contains("doubleSided"))
		j.at("doubleSided").get_to(material.doubleSided);
	else
		material.doubleSided = false;
}

void to_json(json& j, const PbrMetallicRoughness& pbr)
{
	const vec4f& color = pbr.baseColorFactor;
	if(color != defaultBaseColor)
		j["baseColorFactor"] = {color.r, color.g, color.b, color.a};
	if(pbr.metallicFactor != defaultMetallicFactor)
		j["metallicFactor"] = pbr.metallicFactor;
	if(pbr.roughnessFactor != defaultRoughnessFactor)
		j["roughnessFactor"] = pbr.roughnessFactor;
	if(pbr.baseColorTexture.index >= 0)
		j["baseColorTexture"] = pbr.baseColorTexture;
	if(pbr.metallicRoughnessTexture.index >= 0)
		j["metallicRoughnessTexture"] = pbr.metallicRoughnessTexture;
}

void from_json(const json& j, PbrMetallicRoughness& pbr)
{
	if(j.contains("baseColorFactor"))
	{
		std::vector<float> baseColorFactor;
		j.at("baseColorFactor").get_to(baseColorFactor);
		if(baseColorFactor.size() != 4)
			throw std::length_error("expect baseColorFactor to be vec4");
		pbr.baseColorFactor = *reinterpret_cast<const vec4f*>(baseColorFactor.data());
	}
	else
		pbr.baseColorFactor = defaultBaseColor;
	
	if(j.contains("metallicFactor"))
		j.at("metallicFactor").get_to(pbr.metallicFactor);
	else
		pbr.metallicFactor = defaultMetallicFactor;
	
	if(j.contains("roughnessFactor"))
		j.at("roughnessFactor").get_to(pbr.roughnessFactor);
	else
		pbr.roughnessFactor = defaultRoughnessFactor;
	
	if(j.contains("baseColorTexture"))
		from_json(j["baseColorTexture"], pbr.baseColorTexture);
	else
		pbr.baseColorTexture.index = -1;
	
	if(j.contains("metallicRoughnessTexture"))
		from_json(j["metallicRoughnessTexture"], pbr.metallicRoughnessTexture);
	else
		pbr.metallicRoughnessTexture.index = -1;
	
	if(j.contains("baseColorTexture"))
		from_json(j["baseColorTexture"], pbr.baseColorTexture);
	else
		pbr.baseColorTexture.index = -1;
}

static constexpr pea::Primitive defaultPrimitiveMode = pea::Primitive::TRIANGLES;
static constexpr  pea::Primitive endPrimitiveMode = pea::Primitive::QUADRILATERALS;

void to_json(json& j, const glTF2::Primitive& primitive)
{
	j["attributes"] = primitive.attributes;

	if(primitive.indices >= 0)
		j["indices"] = primitive.indices;
	if(primitive.material >= 0)
		j["material"] = primitive.material;
	if(primitive.mode != defaultPrimitiveMode)
		j["mode"] = underlying_cast(primitive.mode);
}

void from_json(const json& j, glTF2::Primitive& primitive)
{
	j.at("attributes").get_to(primitive.attributes);

	int32_t& indices = primitive.indices;
	if(j.contains("indices"))
	{
		j.at("indices").get_to(indices);
		if(indices < 0)
			throw std::out_of_range("invalid indices " + std::to_string(indices));
	}
	else
		indices = -1;
	
	int32_t& material = primitive.material;
	if(j.contains("material"))
	{
		j.at("material").get_to(material);
		if(material < 0)
			throw std::out_of_range("invalid indices " + std::to_string(material));
	}
	else
		material = -1;
	
	pea::Primitive& mode = primitive.mode;
	if(j.contains("mode"))
	{
		mode = endPrimitiveMode;
		j.at("mode").get_to(primitive.mode);
		if(primitive.mode >= endPrimitiveMode)
			throw std::out_of_range("invalid primitive mode" + std::to_string(underlying_cast(mode)));
	}
	else
		primitive.mode = defaultPrimitiveMode;
}

void to_json(json& j, const Mesh& mesh)
{
	assign(j, "name", mesh.name);
	j["primitives"] = mesh.primitives;
	if(!mesh.weights.empty())
		j["weights"] = mesh.weights;
}

void from_json(const json& j, Mesh& mesh)
{
	extract(j, "name", mesh.name);
	extract(j, "weights", mesh.weights);
	j.at("primitives").get_to(mesh.primitives);
}

constexpr quaternionf defaultRotation;
constexpr vec3f defaultTranslation(0, 0, 0);
constexpr vec3f defaultScale(1, 1, 1);

void Node::clearTransformation()
{
	matrix.setIdentity();
	rotation = defaultRotation;
	scale = defaultScale;
	translation = defaultTranslation;
}

void to_json(json& j, const Node& node)
{
	if(!node.name.empty())
		j["name"] = node.name;
	
	if(!node.children.empty())
		j["children"] = node.children;
	if(node.camera >= 0)
		j["camera"] = node.camera;
	if(node.mesh >= 0)
		j["mesh"] = node.mesh;
	
	if(node.matrix != mat4f(1.0f))
	{
		json& matrix = j["matrix"];
		for(int8_t i = 0; i < 4; ++i)
			for(int8_t j = 0; j < 4; ++j)
				matrix.push_back(node.matrix[j][i]);
	}
	const quaternionf& rotation = node.rotation;
	const vec3f& scale = node.scale;
	const vec3f& translation = node.translation;
	if(rotation != defaultRotation)
		j["rotation"] = {rotation.x, rotation.y, rotation.z, rotation.w};
	if(scale != defaultScale)
		j["scale"] = {scale.x, scale.y, scale.z};
	if(translation != defaultTranslation)
		j["translation"] = {translation.x, translation.y, translation.z};
	
}

template <typename T>
static void from_json(const json& j, mat4<T>& m)
{
	std::vector<T> data;
	data.reserve(16);
	j.get_to(data);
	if(data.size() != 16)
		throw std::length_error("expect 16 elements for mat4 type");
	
	for(int8_t i = 0; i < 4; ++i)
		for(int8_t j = 0; j < 4; ++j)
			m[j][i] = data[i * 4 + j];
}

template <typename T>
static void from_json(const json& j, quaternion<T>& m)
{
	std::vector<T> data;
	data.reserve(4);
	j.get_to(data);
	if(data.size() != 4)
		throw std::length_error("expect 4 elements for quaternion type");
	
	m.x = data[0];
	m.y = data[1];
	m.z = data[2];
	m.w = data[3];
}

template <typename T>
static void from_json(const json& j, vec3<T>& v)
{
	std::vector<T> data;
	data.reserve(3);
	j.get_to(data);
	if(data.size() != 3)
		throw std::length_error("expect 3 elements for vec3 type");
	
	v = *reinterpret_cast<const vec3f*>(data.data());
}

void from_json(const json& j, Node& node)
{
	if(j.contains("name"))
		j.at("name").get_to(node.name);
	else
		node.name.clear();
	
	if(j.contains("children"))
		j.at("children").get_to(node.children);
	else
		node.children.clear();
	
	extract(j, "camera", node.camera);
	extract(j, "mesh", node.mesh);
	
	mat4f& matrix = node.matrix;
	if(j.contains("matrix"))
		from_json(j["matrix"], matrix);
	else
		matrix.setIdentity();
	
	if(j.contains("rotation"))
		from_json(j["rotation"], node.rotation);
	else
		node.rotation = defaultRotation;
	
	if(j.contains("scale"))
		from_json(j["scale"], node.scale);
	else
		node.scale = defaultScale;
	
	if(j.contains("translation"))
		from_json(j["translation"], node.translation);
	else
		node.translation = defaultTranslation;
}

void to_json(json& j, const Scene& scene)
{
	if(!scene.name.empty())
		j["name"] = scene.name;
	if(!scene.nodes.empty())
		j["nodes"] = scene.nodes;
}

void from_json(const json& j, Scene& scene)
{
	if(j.contains("name"))
		j["name"].get_to(scene.name);
	else
		scene.name.clear();
	
	if(j.contains("nodes"))
		j["nodes"].get_to(scene.nodes);
}

void to_json(json& j, const glTF& model)
{
#if __cplusplus >= 202002L
	auto assign = [&j]<typename T>(const std::string& key, const std::vector<T>& v)
#else
	auto assign = [&j](const std::string& key, const auto& v)
#endif
	{
		if(!v.empty())
			j[key] = v;
	};

	j["asset"] = model.asset;
	assign("buffers", model.buffers);
	assign("bufferViews", model.bufferViews);
	assign("accessors", model.accessors);
	assign("images", model.images);
	assign("textures", model.textures);
	assign("cameras", model.cameras);
	assign("materials", model.materials);
	assign("meshes", model.meshes);
	assign("nodes", model.nodes);
	assign("scenes", model.scenes);
	assign("cameras", model.cameras);
	glTF2::assign(j, "scene", model.scene);
}

void from_json(const json& j, glTF& model)
{
#if __cplusplus >= 202002L
	auto extract = [&j]<typename T>(const std::string& key, std::vector<T>& v)
#else
	auto extract = [&j](const std::string& key, auto& v)
#endif
	{
		if(j.contains(key))
			j.at(key).get_to(v);
	};
	j.at("asset").get_to(model.asset);

	extract("buffers", model.buffers);
	extract("bufferViews", model.bufferViews);
	extract("accessors", model.accessors);
	extract("images", model.images);
	extract("textures", model.textures);
	extract("materials", model.materials);
	extract("meshes", model.meshes);
	extract("nodes", model.nodes);
	extract("scenes", model.scenes);
	extract("cameras", model.cameras);
	glTF2::extract(j, "scene", model.scene);
}

}  // namespace glTF2

Model_glTF2::Model_glTF2(const std::string& path) noexcept(false)
{
	std::ifstream file(path);
	json j;
	file >> j;
	from_json(j, model);
}

bool Model_glTF2::save(const std::string& path, uint32_t space/* = 4*/) const
{
	json j;
	to_json(j, model);
	
	std::ofstream file(path);
	file << std::setw(space) << j << std::endl;
	return true;
}

}  // namespace pea