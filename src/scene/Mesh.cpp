#include "scene/Mesh.h"

#include <algorithm>
#include <cassert>
#include <cinttypes>

#include "io/Group.h"
#include "math/hash.h"
#include "opengl/Shader.h"
#include "opengl/Program.h"
#include "opengl/GL.h"
#include "scene/Material.h"
#include "util/compiler.h"
#include "util/utility.h"
#include "util/Log.h"


static const char* TAG = "Mesh";

// why offset is a pointer, not a integer in OpenGL functions, such as glEnableVertexAttribArray
// do you think ptr = ptr1 + ptr2, or ptr = ptr1 + offset ???
static inline const void* offset(int x)
{
	return reinterpret_cast<const void*>(x);
}

using namespace pea;

Mesh::Mesh():
		material(nullptr),
//		aabb(),
		vao(0),
		vbo{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		primitive(Primitive::TRIANGLES),
		viewPositionLocation(Shader::INVALID_LOCATION)
{
//	std::fill(vbo, vbo + sizeofArray(vbo), 0);
}

Mesh::Mesh(const Mesh& mesh):
		name(mesh.name),
		material(mesh.material),
//		aabb(mesh.aabb),
		vao(0),
		vbo{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		primitive(mesh.primitive),
		viewPositionLocation(mesh.viewPositionLocation)
{
	this->vertices  = mesh.vertices;
	this->texcoords = mesh.texcoords;
	this->normals   = mesh.normals;
	this->indices   = mesh.indices;
//	this->material  = mesh.material;
}

Mesh::Mesh(Mesh&& mesh):
		name(mesh.name),
		material(mesh.material),
//		aabb(mesh.aabb),
		vao(mesh.vao),
		primitive(mesh.primitive),
		viewPositionLocation(mesh.viewPositionLocation)
{
//	mesh.aabb.reset();
	for(size_t i = 0; i < sizeofArray(vbo); ++i)
	{
		this->vbo[i] = mesh.vbo[i];
		mesh.vbo[i] = 0u;;
	}
	
	this->vertices  = std::move(mesh.vertices);
	this->colors    = std::move(mesh.colors);
	this->texcoords = std::move(mesh.texcoords);
	this->normals   = std::move(mesh.normals);
	this->indices   = std::move(mesh.indices);
//	this->material  = std::move(mesh.material);
}

Mesh& Mesh::operator =(const Mesh& mesh)
{
	if(this != &mesh)
	{
		this->name = mesh.name;
		this->material = mesh.material;
//		this->aabb = mesh.aabb;
		
		// DONT copy VAO & VBO state.
		glDeleteBuffers(sizeofArray(vbo), vbo);
		std::fill(vbo, vbo + sizeofArray(vbo), 0u);
		
		glDeleteVertexArrays(1, &vao);
		vao = 0u;

		this->primitive = mesh.primitive;
		this->viewPositionLocation = mesh.viewPositionLocation;
		
		this->vertices  = mesh.vertices;
		this->colors    = mesh.colors;
		this->texcoords = mesh.texcoords;
		this->normals   = mesh.normals;
		this->indices   = mesh.indices;
//		this->material  = mesh.material;
	}
	return *this;
}

Mesh& Mesh::operator =(Mesh&& mesh)
{
	this->name = mesh.name;
	
	glDeleteBuffers(sizeofArray(vbo), this->vbo);
	for(size_t i = 0; i < sizeofArray(vbo); ++i)
	{
		this->vbo[i] = mesh.vbo[i];
		mesh.vbo[i] = 0u;;
	}
	
	glDeleteVertexArrays(1, &vao);
	this->vao = mesh.vao;
	mesh.vao = 0u;
	
	this->primitive = mesh.primitive;
	this->viewPositionLocation = mesh.viewPositionLocation;
	
	this->vertices  = std::move(mesh.vertices);
	this->texcoords = std::move(mesh.texcoords);
	this->colors    = std::move(mesh.colors);
	this->normals   = std::move(mesh.normals);
	this->indices   = std::move(mesh.indices);
	this->material  = std::move(mesh.material);
	return *this;
}

Mesh::~Mesh()
{
	glDeleteBuffers(sizeofArray(vbo), vbo);
	glDeleteVertexArrays(1, &vao);
}

void Mesh::applyTransform()
{
	mat4f transform = getTransform();
	mat3f rs;
	for(uint8_t i = 0; i < 9; ++i)
		rs[i / 3][i % 3] = transform[i / 3][i % 3];
//	vec3f translation(transform[]);

	for(vec3f& vertex: vertices)
	{
		vec4f position(vertex.x, vertex.y, vertex.z, 1);
		position = transform * position;
		vertex = *reinterpret_cast<vec3f*>(&position);
	}
	
	rs.inverse().transpose();
	for(vec3f& normal: normals)
		normal = rs * normal;
	
	transform.identity();
	setTransform(transform);
}

void Mesh::computeTangentBasis()
{
	assert(!vertices.empty() && !texcoords.empty() && !normals.empty());
	assert(indices.empty());  // compute per vertex attributes
	
	const size_t vertexSize = vertices.size();
	tangents.reserve(vertexSize);
	bitangents.reserve(vertexSize);
	for(size_t i = 0; i < vertexSize; i += 3)
	{
		const vec3f* v = vertices.data() + i;
		const vec2f* t = texcoords.data() + i;
		
		vec3f v01 = v[1] - v[0];
		vec3f v02 = v[2] - v[0];
		
		vec2f t01 = t[1] - t[0];
		vec2f t02 = t[2] - t[0];
		
		// project v01, v02 into TBN tangent space
		// v01 = t01.s * T + t01.t * B;
		// v02 = t02.s * T + t02.t * B;
		float det = t01.s * t02.t - t01.t * t02.s;
		vec3f tangent = (v01 * t02.t - v02 * t01.t) / det;
		vec3f bitangent = (v02 * t01.s - v01 * t02.s) / det;
		
		for(uint8_t j = 0; j < 3; ++j)
		{
			tangents.push_back(tangent);
			bitangents.push_back(bitangent);
		}
	}
}

bool Mesh::hasIndex() const
{
	return indices.empty();
}

BoundingBox Mesh::getBoundingBox() const
{
	BoundingBox box;
	for(const vec3f& vertex: vertices)
		box.add(vertex);

	return box;
}

void Mesh::reverseWinding()
{
	for(size_t i = 0, size = indices.size(); i < size; i += 3)
		std::swap(indices[i + 1], indices[i + 2]);

	// flip vertex and face normals
	for(vec3f& normal: normals)
		normal = -normal;
	for(vec3f& normal: faceNormals)
		normal = -normal;

}

bool Mesh::verifyBone() const
{
	size_t vertexCount = vertices.size();
	std::vector<float> sum(vertexCount, 0.0);
	for(const Bone& bone: bones)
	{
		const std::vector<VertexWeight>& weights = bone.weights;
		for(VertexWeight v: weights)
			sum[v.index] += v.weight;
	}

	bool valid = true;
	for(size_t i = 0; i < vertexCount; ++i)
	{
		if(std::abs(sum[i] - 1.0) > 0.001)  // tolerance
		{
			slog.w(TAG, "vertex id(%zu)'s bone weight sum(%.6f) != 1", i, sum[i]);
			valid = false;
		}
	}
	return valid;
}

void Mesh::prepare(Primitive primitive/* = Primitive::TRIANGLES*/)
{
	slog.v(TAG, "primitive=%d, vao=%u, vertices.size=%zu", primitive, vao, vertices.size());
	this->primitive = primitive;
	
	assert(vao == 0);  // prepare() has been called many times.
	glGenVertexArrays(1, &vao);
	
	assert(!vertices.empty());
	glGenBuffers(sizeofArray(vbo), vbo);
	
	const uint32_t& vbo_position = vbo[Shader::ATTRIBUTE_VEC_POSITION];
	const uint32_t& vbo_normal   = vbo[Shader::ATTRIBUTE_VEC_NORMAL];
	const uint32_t& vbo_texcoord = vbo[Shader::ATTRIBUTE_VEC_TEXCOORD];
	const uint32_t& vbo_color    = vbo[Shader::ATTRIBUTE_VEC_COLOR];
	const uint32_t& vbo_index    = vbo[Shader::ATTRIBUTE_INT_INDEX];
	
	slog.d(TAG, "name=%s, vao=%" PRIu32 ", vbo: vertex=%" PRIu32 ", normal=%" PRIu32
			", texcoord=%" PRIu32 ", color=%" PRIu32 ", index=%" PRIu32 ", primitive=0x%X", name.c_str(), vao, 
			vbo_position, vbo_normal, vbo_texcoord, vbo_color, vbo_index, primitive);
}

void Mesh::upload() const
{
	assert(vao != 0);
	glBindVertexArray(vao);

	assert(vbo[Shader::ATTRIBUTE_VEC_POSITION] != 0);  // prepare() needs to be called before.
	slog.d(TAG, "size: vertex=%zu, color=%zu, texcoord=%zu, normal=%zu, index=%zu",
			vertices.size(), colors.size(), texcoords.size(), normals.size(), indices.size());
	GL::bindVertexBuffer(vbo[Shader::ATTRIBUTE_VEC_POSITION], Shader::ATTRIBUTE_VEC_POSITION, vertices);
	
	auto bindVertexBuffer = [&vbo = vbo](uint32_t attributeIndex, const std::vector<vec3f> data)
	{
		if(!data.empty())
			GL::bindVertexBuffer(vbo[attributeIndex], attributeIndex, data);
	};
	
	bindVertexBuffer(Shader::ATTRIBUTE_VEC_COLOR, colors);
	bindVertexBuffer(Shader::ATTRIBUTE_VEC_NORMAL, normals);
	
	if(!texcoords.empty())
		GL::bindVertexBuffer(vbo[Shader::ATTRIBUTE_VEC_TEXCOORD], Shader::ATTRIBUTE_VEC_TEXCOORD, texcoords);
	
	bindVertexBuffer(Shader::ATTRIBUTE_VEC_TANGENT, tangents);
	bindVertexBuffer(Shader::ATTRIBUTE_VEC_BITANGENT, bitangents);
	
	if(!indices.empty())
		GL::bindIndexBuffer(vbo[Shader::ATTRIBUTE_INT_INDEX], indices);
	
	if(!instances0.empty())
	{
		uint32_t startIndex = Shader::ATTRIBUTE_VEC_INSTANCE;
		GL::bindVertexBuffer(vbo[startIndex], startIndex, instances0);
		glVertexAttribDivisor(Shader::ATTRIBUTE_VEC_INSTANCE, 1);
	}
	else if(!instances1.empty())
	{
		uint32_t startIndex = Shader::ATTRIBUTE_MAT_INSTANCE;
		GL::bindVertexBuffer(vbo[startIndex], startIndex, instances1);
		for(uint32_t i = 0; i < 4; ++i)  // a mat4 attribute takes up 4 attribute locations
			glVertexAttribDivisor(startIndex + i, 1);
	}
	
	// non-constant condition for static assertion
//	static_assert(sizeofArray(vbo) == Shader::ATTRIBUTE_INT_INDEX + 1);
	static_assert(sizeof(vbo) / sizeof(vbo[0]) == Shader::ATTRIBUTE_INT_INDEX + 1);
		
	glBindVertexArray(0);
}
/*
void Mesh::setMaterial(std::shared_ptr<Material> material)
{
	this->material = material; 
	slog.d(TAG, "setMaterial mesh %s, count=%d, this.count=%d", name.c_str(), (int32_t)material.use_count(),
			(int32_t)this->material.use_count());
}
*/
void Mesh::addLight(Light* light)
{
	assert(light != nullptr);
	lights.push_back(light);
}

void Mesh::removeLight(const Light* light)
{
	assert(light != nullptr);
	auto it = std::find(lights.begin(), lights.end(), light);
	if(it != lights.end())
		lights.erase(it);
}

void Mesh::setProgram(uint32_t program)
{
	Object::setProgram(program);
	
	if(!texcoords.empty())
		assert(glGetAttribLocation(program, "texcoord") == Shader::ATTRIBUTE_VEC_TEXCOORD);
	
	if(!colors.empty())
		assert(glGetAttribLocation(program, "color") == Shader::ATTRIBUTE_VEC_COLOR);
	
	if(!normals.empty())
		assert(glGetAttribLocation(program, "normal") == Shader::ATTRIBUTE_VEC_NORMAL);
	
	if(!instances1.empty())
		assert(glGetAttribLocation(program, "instance") == Shader::ATTRIBUTE_MAT_INSTANCE);
	else if(!instances0.empty())
		assert(glGetAttribLocation(program, "instance") == Shader::ATTRIBUTE_VEC_INSTANCE);
	else
		assert(glGetUniformLocation(program, "model") == Shader::UNIFORM_MAT_MODEL);
	
	viewPositionLocation = glGetUniformLocation(program, "viewPosition");
	assert(viewPositionLocation == Shader::INVALID_LOCATION || viewPositionLocation == Shader::UNIFORM_VEC_VIEW_POSITION);
}

void Mesh::render(const mat4f& viewProjection) const
{
//	Object::render(viewProjection);
	uint32_t program = getProgram();
	assert(program != 0);
	Program::use(program);
	
	const int32_t instanceCount = instances0.size() > 0? instances0.size(): instances1.size();
	if(instanceCount == 0)
		Program::setUniform(Shader::UNIFORM_MAT_MODEL, getTransform());
	Program::setUniform(Shader::UNIFORM_MAT_VIEW_PROJECTION, viewProjection);
	
	uint32_t textureIndices[5] = {0};
	auto& [diffuseIndex, specularIndex, normalIndex, heightIndex, defaultIndex] = textureIndices;

	uint32_t textureIndex = 0;
	for(const std::shared_ptr<Texture>& texture: textures)
	{
		assert(texture.get() != nullptr);
		
		std::string textureName = "texture";
		switch(texture->getType())
		{
		case Texture::Type::DIFFUSE:
			textureName = "Diffuse" + std::to_string(diffuseIndex);
			++diffuseIndex;
			break;
		case Texture::Type::SPECULAR:
			textureName = "Specular" + std::to_string(specularIndex);
			++specularIndex;
			break;
		case Texture::Type::NORMAL:
			textureName = "Normal" + std::to_string(normalIndex);
			++normalIndex;
			break;
		case Texture::Type::HEIGHT:
			textureName = "Height" + std::to_string(heightIndex);
			++heightIndex;
			break;
		case Texture::Type::NONE:
			textureName = std::to_string(defaultIndex);
			++defaultIndex;
			break;
		default:
			assert(false);
			break;
		}
		
		uint32_t location = glGetUniformLocation(program, textureName.c_str());
		texture->bind(location, textureIndex);
		++textureIndex;
	}
	
//	slog.d(TAG, "setMaterial mesh %s, count=%d, this.count=%d", name.c_str(), (int32_t)material.use_count(),
//			(int32_t)this->material.use_count());
//	assert(material.get() != nullptr);
	if(material != nullptr)
		Program::setMaterial(program, "material", *material);

	for(size_t i = 0, size = lights.size(); i < size; ++i)
	{
		std::string lightName = "light" + std::to_string(i);
//		slog.v(TAG, "enable %s", lightName.c_str());
		const Light* light = lights[i];
		assert(light);
		Program::setLight(program, lightName.c_str(), *light);
	}
	if(viewPositionLocation != Shader::INVALID_LOCATION)
		Program::setUniform(Shader::UNIFORM_VEC_VIEW_POSITION, viewPosition);
	
//	Program::setUniform(Shader::UNIFORM_MAT_VIEW_PROJECTION, viewProjection);
	GLenum mode = GL::getPrimitive(primitive);
	// TODO: primitive restart
//	slog.i(TAG, "vertices.size()=%zu, normals.size()=%zu, indices.size()=%zu, mode=0x%X", vertices.size(), normals.size(), indices.size(), mode);
	assert(vao != 0);
	glBindVertexArray(vao);
	const int32_t indexCount = indices.size();
	if(instanceCount > 0)  // instance rendering
	{
		if(indexCount > 0)
			glDrawElementsInstanced(mode, indexCount, GL_UNSIGNED_INT, nullptr, instanceCount);
		else
			glDrawArraysInstanced(mode, 0, vertices.size(), instanceCount);
	}
	else
	{
		if(indexCount > 0)
			glDrawElements(mode, indexCount, GL_UNSIGNED_INT, nullptr);
		else
			glDrawArrays(mode, 0, vertices.size());
	}

#ifndef NDEBUG
	static bool printed = false;
	if(!printed)
	{
		const char* PRIMITIVE_STRINGS[] =
		{
			"points",
			"lines", "line_loop", "line_strip",
			"triangles", "triangle_strip", "triangle_fan",
			"quadrilaterals", "quadrilateral_strip",
			"polygon"
		};
		const char* mode = PRIMITIVE_STRINGS[underlying_cast<Primitive>(primitive)];
		if(instanceCount > 0)
		{
			if(indexCount > 0)
				slog.d(TAG, "glDrawElementsInstanced(%s), size=%zu, instance=%zu", mode, indexCount, instanceCount);
			else
				slog.d(TAG, "glDrawArraysInstanced(%s), size=%zu, instance=%zu", mode, vertices.size(), instanceCount);
		}
		else
		{
			if(indexCount > 0)
				slog.d(TAG, "glDrawElements(%s), size=%zu", mode, indexCount);
			else
				slog.d(TAG, "glDrawArrays(%s), size=%zu", mode, vertices.size());
		}

		printed = true;
	}
#endif
	glBindVertexArray(0);
}

Mesh::Builder::Builder(const std::vector<vec3f>& vertices):
		vertices(vertices)
{
	assert(!vertices.empty());
}

Mesh::Builder& Mesh::Builder::setTexcoord(const std::vector<vec2f>& texcoords)
{
	assert(vertices.size() == texcoords.size());
	this->texcoords = texcoords;
	return *this;
}

Mesh::Builder& Mesh::Builder::setColor(const std::vector<vec3f>& colors)
{
	assert(vertices.size() == colors.size());
	this->colors = std::move(colors);
	return *this;
}

Mesh::Builder& Mesh::Builder::setNormal(const std::vector<vec3f>& normals)
{
	assert(vertices.size() == normals.size());
	this->normals = normals;
	return *this;
}

Mesh::Builder& Mesh::Builder::setInstance(const std::vector<vec4f>& instances)
{
	instances0 = instances;
	return *this;
}

Mesh::Builder& Mesh::Builder::setInstance(const std::vector<mat4f>& instances)
{
	instances1 = instances;
	return *this;
}

Mesh::Builder& Mesh::Builder::setIndex(const std::vector<uint32_t>& indices)
{
#ifndef NDEBUG
	verifyIndex(indices);
#endif
	this->indices = indices;
	return *this;
}
	
Mesh::Builder::Builder(std::vector<vec3f>&& vertices):
		vertices(std::move(vertices))
{
	assert(!this->vertices.empty());
}

Mesh::Builder& Mesh::Builder::setTexcoord(std::vector<vec2f>&& texcoords)
{
	assert(vertices.size() == texcoords.size());
	this->texcoords = std::move(texcoords);
	return *this;
}

Mesh::Builder& Mesh::Builder::setColor(std::vector<vec3f>&& colors)
{
	assert(vertices.size() == colors.size());
	this->colors = std::move(colors);
	return *this;
}

Mesh::Builder& Mesh::Builder::setNormal(std::vector<vec3f>&& normals)
{
	assert(vertices.size() == normals.size());
	this->normals = std::move(normals);
	return *this;
}

Mesh::Builder& Mesh::Builder::setInstance(std::vector<vec4f>&& instances)
{
	instances0 = std::move(instances);
	return *this;
}

Mesh::Builder& Mesh::Builder::setInstance(std::vector<mat4f>&& instances)
{
	instances1 = std::move(instances);
	return *this;
}

Mesh::Builder& Mesh::Builder::setIndex(std::vector<uint32_t>&& indices)
{
#ifndef NDEBUG
	verifyIndex(indices);
#endif
	this->indices = std::move(indices);
	return *this;
}

void Mesh::Builder::verifyIndex(const std::vector<uint32_t>& indices) const
{
	uint32_t vertexCount = vertices.size();
	for(const uint32_t& index: indices)
	{
		if(index < vertexCount)  // [[likely]]
			continue;
		
		constexpr char _ = ' ';
		std::ostringstream oss;
		oss << "index" << _ << index << _ << "out of range" << _ 
				<< '[' << 0 << ',' << _ << vertexCount << ']';
		std::string message = oss.str();
		slog.e(TAG, "#%d error: %s", __LINE__, message.c_str());
		throw std::invalid_argument(message);
	}
}

class Vertex
{
protected:
	static constexpr float POSITION_TOLERANCE = 1E-3;  // 1 mm
	static constexpr float TEXCOORD_TOLERANCE = 1.0F / 2048;  // texture size 2048
	static constexpr float COLOR_TOLERANCE    = 1.0F / 1024;  // 10 bit color depth
	static constexpr float VECTOR_TOLERANCE   = 1E-4;
	
public:
	vec3f position;
	vec2f texcoord;
	vec3f color;
	vec3f normal;
	
public:
	size_t hash() const;
	friend bool operator ==(const Vertex& lhs, const Vertex& rhs);
	
};

size_t Vertex::hash() const
{
	size_t hashValue = std::hash<vec3f>()(position);
	hash_combine(hashValue, texcoord);
	hash_combine(hashValue, color);
	hash_combine(hashValue, normal);
	return hashValue;
}

static bool isEqual(const vec2f& v, const float& tolerance)
{
	vec2f a = abs(v);
	return a.x <= tolerance && a.y <= tolerance;
}

static bool isEqual(const vec3f& v, const float& tolerance)
{
	vec3f a = abs(v);
	return a.x <= tolerance && a.y <= tolerance && a.z <= tolerance;
}

bool operator ==(const Vertex& lhs, const Vertex& rhs)
{
	return isEqual(lhs.position - rhs.position, Vertex::POSITION_TOLERANCE) &&
			isEqual(lhs.texcoord - rhs.texcoord, Vertex::TEXCOORD_TOLERANCE) &&
			isEqual(lhs.color - rhs.color, Vertex::COLOR_TOLERANCE) &&
			isEqual(lhs.normal - rhs.normal, Vertex::VECTOR_TOLERANCE);
}

class VertexTangent: public Vertex
{
public:
	vec3f tangent;
	vec3f bitangent;

public:
	size_t hash() const;
	
	friend bool operator ==(const VertexTangent& lhs, const VertexTangent& rhs);

};

size_t VertexTangent::hash() const
{
	size_t hashValue = Vertex::hash();
	hash_combine(hashValue, tangent);
	hash_combine(hashValue, bitangent);
	return hashValue;
}
	
bool operator ==(const VertexTangent& lhs, const VertexTangent& rhs)
{
	return static_cast<const Vertex&>(lhs) == static_cast<const Vertex&>(rhs) &&
			isEqual(lhs.tangent - rhs.tangent, VertexTangent::VECTOR_TOLERANCE) &&
			isEqual(lhs.bitangent - rhs.bitangent, VertexTangent::VECTOR_TOLERANCE);
}

bool Mesh::Builder::isIndexed() const
{
	return !indices.empty();
}

template <typename T>
static void createIndex(std::vector<T>& data, const std::vector<uint32_t>& indexMap)
{
	if(data.empty())
		return;
	
	const uint32_t size = data.size();
	std::vector<T> _data;
	_data.reserve(size);
	_data.push_back(data[0]);
	
	for(uint32_t i = 1, max = 0; i < size; ++i)
	{
		if(indexMap[i] > max)
		{
			max = indexMap[i];
			_data.push_back(data[i]);
		}
	}
	
	data = std::move(_data);
}

Mesh::Builder& Mesh::Builder::shrinkToIndex()
{
	assert(indices.empty());

	const uint32_t size = vertices.size();
	std::vector<uint32_t> indexMap(size);  // keep insertion order
	uint32_t shrinkedSize;

	const vec2f ZERO2(0, 0);
	const vec3f ZERO3(0, 0, 0);
	bool tangentSpace = !tangents.empty() && !bitangents.empty();
	if(tangentSpace)  // TODO: use template?
	{
		auto hash = [](const VertexTangent& vertex) { return vertex.hash(); };
		auto compare = [](const VertexTangent& lhs, const VertexTangent& rhs) { return lhs == rhs; };
		std::unordered_map<VertexTangent, uint32_t,
				decltype(hash), decltype(compare)
				> vertexMap(size, hash, compare);
	
		for(uint32_t i = 0, j = 0; i < size; ++i)
		{
			VertexTangent vertex;
			vertex.position = vertices[i];
			vertex.texcoord = texcoords.empty()? ZERO2: texcoords[i];
			vertex.color    = colors.empty()   ? ZERO3: colors[i];
			vertex.normal   = normals.empty()  ? ZERO3: normals[i];
			vertex.tangent  = tangents[i];
			vertex.bitangent= bitangents[i];
			
			auto it = vertexMap.find(vertex);
			if(it == vertexMap.end())
			{
				vertexMap.emplace_hint(it, vertex, j);
				indexMap[i] = j;
				++j;
			}
			else
				indexMap[i] = it->second;
		}
		
		shrinkedSize = indexMap.size();
	}
	else
	{
		auto hash = [](const Vertex& vertex) { return vertex.hash(); };
		auto compare = [](const Vertex& lhs, const Vertex& rhs) { return lhs == rhs; };
		std::unordered_map<Vertex, uint32_t,
				decltype(hash), decltype(compare)
				> vertexMap(size, hash, compare);
	
		for(uint32_t i = 0, j = 0; i < size; ++i)
		{
			Vertex vertex;
			vertex.position = vertices[i];
			vertex.texcoord = texcoords.empty()? ZERO2: texcoords[i];
			vertex.color    = colors.empty()   ? ZERO3: colors[i];
			vertex.normal   = normals.empty()  ? ZERO3: normals[i];
			
			auto it = vertexMap.find(vertex);
			if(it == vertexMap.end())
			{
				vertexMap.emplace_hint(it, vertex, j);
				indexMap[i] = j;
				++j;
			}
			else
				indexMap[i] = it->second;
		}
		
		shrinkedSize = indexMap.size();
	}

	createIndex(vertices  , indexMap);
	createIndex(texcoords , indexMap);
	createIndex(colors    , indexMap);
	createIndex(normals   , indexMap);
	createIndex(tangents  , indexMap);
	createIndex(bitangents, indexMap);
	
	if(indices.empty())
		indices = std::move(indexMap);
	else
	{
		for(uint32_t& index: indices)
			index = indexMap[index];
	}
	
	return *this;
}

Mesh::Builder& Mesh::Builder::removeIndex()
{
	assert(!indices.empty());
	
	Group::dropIndex(vertices, indices);
	Group::dropIndex(texcoords, indices);
	Group::dropIndex(colors, indices);
	
	Group::dropIndex(normals, indices);
	Group::dropIndex(tangents, indices);
	Group::dropIndex(bitangents, indices);
	
	indices.clear();
	return *this;
}

std::unique_ptr<Mesh> Mesh::Builder::build()
{
	std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();
	
	mesh->vertices = std::move(vertices);
	mesh->colors   = std::move(colors);
	mesh->normals  = std::move(normals);
	mesh->texcoords= std::move(texcoords);
	
	if(!instances1.empty())
	{
		mesh->instances1 = std::move(instances1);
		if(!instances0.empty())
			slog.w(TAG, "setInstance() called with vec4 and mat4 types, vec4 will be overlooked");
	}
	else
		mesh->instances0 = std::move(instances0);
	
	mesh->indices  = std::move(indices);

	return mesh;
}
