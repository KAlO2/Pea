#include "scene/Mesh.h"

#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <cstring>

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
		viewProjectionLocation(Shader::INVALID_LOCATION)
{
//	std::fill(vbo, vbo + sizeofArray(vbo), 0);
}

Mesh::Mesh(const Mesh& mesh):
		name(mesh.name),
		material(mesh.material),
//		aabb(mesh.aabb),
		vao(0),
		vbo{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		primitive(mesh.primitive)
{
	this->vertices  = mesh.vertices;
	this->positions = mesh.positions;
	this->texcoords = mesh.texcoords;
	this->normals   = mesh.normals;
	this->indices   = mesh.indices;
//	this->material  = mesh.material;
	this->uniformBlock = mesh.uniformBlock;
}

Mesh::Mesh(Mesh&& mesh):
		name(mesh.name),
		material(mesh.material),
//		aabb(mesh.aabb),
		vao(mesh.vao),
		primitive(mesh.primitive)
{
//	mesh.aabb.reset();
	for(size_t i = 0; i < sizeofArray(vbo); ++i)
	{
		this->vbo[i] = mesh.vbo[i];
		mesh.vbo[i] = 0u;;
	}
	
	this->vertices  = std::move(mesh.vertices);
	this->positions = std::move(mesh.positions);
	this->colors    = std::move(mesh.colors);
	this->texcoords = std::move(mesh.texcoords);
	this->normals   = std::move(mesh.normals);
	this->indices   = std::move(mesh.indices);
//	this->material  = std::move(mesh.material);
	this->uniformBlock = std::move(mesh.uniformBlock);
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
		
		this->vertices  = mesh.vertices;
		this->positions = mesh.positions;
		this->colors    = mesh.colors;
		this->texcoords = mesh.texcoords;
		this->normals   = mesh.normals;
		this->indices   = mesh.indices;
//		this->material  = mesh.material;
		this->uniformBlock = mesh.uniformBlock;
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
	
	this->vertices  = std::move(mesh.vertices);
	this->positions = std::move(mesh.positions);
	this->texcoords = std::move(mesh.texcoords);
	this->colors    = std::move(mesh.colors);
	this->normals   = std::move(mesh.normals);
	this->indices   = std::move(mesh.indices);
//	this->material  = std::move(mesh.material);
	this->uniformBlock = std::move(mesh.uniformBlock);
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

	const bool usePosition = !positions.empty();
	vec3f* data = usePosition? positions.data(): reinterpret_cast<vec3f*>(vertices.data());
	const size_t size = usePosition? positions.size(): vertices.size();
	const size_t advance = usePosition? 3: 4;
	for(size_t i = 0; i < size; ++i, data += advance)
	{
		vec4f position(data->x, data->y, data->z, 1.0F);
		position = transform * position;
		data->x = position.x;
		data->y = position.y;
		data->z = position.z;
	}
	
	rs.inverse().transpose();
	for(vec3f& normal: normals)
		normal = rs * normal;
	
	transform.identity();
	setTransform(transform);
}

void Mesh::computeTangentBasis()
{
	assert(!texcoords.empty() && !normals.empty());
	assert(indices.empty());  // compute per vertex attributes
	
	const bool usePosition = !positions.empty(); 
	const size_t vertexSize = usePosition? positions.size(): vertices.size();
	tangents.reserve(vertexSize);
	bitangents.reserve(vertexSize);
	for(size_t i = 0; i < vertexSize; i += 3)
	{
		const vec3f* v;
		if(usePosition)
			v = positions.data() + i;
		else
			v = reinterpret_cast<const vec3f*>(vertices.data() + i);
		
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

size_t Mesh::getVertexSize() const
{
	size_t size = positions.size();  // mostly vec3 position
	if(size > 0)
		return size;
	return vertices.size();
}

size_t Mesh::getIndexSize() const
{
	return indices.size();
}

BoundingBox Mesh::getBoundingBox() const
{
	BoundingBox box;
	if(!positions.empty())
		for(const vec3f& position: positions)
			box.add(position);
	else
		for(const vec4f& vertex: vertices)
			box.add(vec3f(vertex.x, vertex.y, vertex.z));

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
	const size_t vertexCount = getVertexSize();
/*	TODO:
	std::vector<float> sum(vertexCount, 0.0);
	for(const Bone& bone: bones)
	{
		const std::vector<VertexWeight>& weights = bone.weights;
		for(VertexWeight v: weights)
			sum[v.index] += v.weight;
	}
*/
	bool valid = true;
/*	for(size_t i = 0; i < vertexCount; ++i)
	{
		if(std::abs(sum[i] - 1.0) > 1E-6)  // tolerance
		{
			slog.w(TAG, "vertex id(%zu)'s bone weight sum(%.6f) != 1", i, sum[i]);
			valid = false;
		}
	}
*/
	return valid;
}

void Mesh::prepare(Primitive primitive/* = Primitive::TRIANGLES*/)
{
	slog.v(TAG, "primitive=%d, vao=%u, vertices.size=%zu", primitive, vao, getVertexSize());
	this->primitive = primitive;
	
	assert(vao == 0);  // if failed, prepare() has been called before.
	glGenVertexArrays(1, &vao);
	
	assert(!vertices.empty() || !positions.empty());
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

uint32_t Mesh::getVertexBufferObject(uint32_t attributeIndex) const
{
	assert(attributeIndex < sizeofArray(vbo));
	return vbo[attributeIndex];
}

void Mesh::upload() const
{
	assert(vao != 0);
	glBindVertexArray(vao);

	assert(vbo[Shader::ATTRIBUTE_VEC_POSITION] != 0);  // prepare() needs to be called before.
	slog.d(TAG, "size: vertex=%zu, color=%zu, texcoord=%zu, normal=%zu, index=%zu",
			getVertexSize(), colors.size(), texcoords.size(), normals.size(), indices.size());
	
	auto bindVertexBuffer = [&vbo = vbo](uint32_t attributeIndex, const std::vector<vec3f> data)
	{
		if(!data.empty())
			GL::bindVertexBuffer(vbo[attributeIndex], attributeIndex, data);
	};
	
	if(vertices.empty())
		GL::bindVertexBuffer(vbo[Shader::ATTRIBUTE_VEC_VERTEX], Shader::ATTRIBUTE_VEC_VERTEX, vertices);
	bindVertexBuffer(Shader::ATTRIBUTE_VEC_POSITION, positions);
	
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

void Mesh::upload(uint32_t vboFlag[Mesh::VBO_COUNT]) const
{
	assert(vao != 0);
	glBindVertexArray(vao);

	assert(vbo[Shader::ATTRIBUTE_VEC_POSITION] != 0);  // prepare() needs to be called before.
	slog.d(TAG, "size: vertex=%zu, color=%zu, texcoord=%zu, normal=%zu, index=%zu",
			getVertexSize(), colors.size(), texcoords.size(), normals.size(), indices.size());
	
	auto bindVertexBuffer = [&vbo = vbo, &vboFlag](uint32_t attributeIndex, const std::vector<vec3f> data)
	{
		if(!data.empty())
			GL::bindVertexBuffer(vbo[attributeIndex], attributeIndex, data, vboFlag[attributeIndex]);
	};
	
	if(vertices.empty())
		GL::bindVertexBuffer(vbo[Shader::ATTRIBUTE_VEC_VERTEX], Shader::ATTRIBUTE_VEC_VERTEX, vertices);
	bindVertexBuffer(Shader::ATTRIBUTE_VEC_POSITION, positions);
	
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

uint32_t Mesh::getVertexBufferObject(int32_t location) const
{
	assert(0 <= location && location < VBO_COUNT);
	return vbo[location];
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

static const char* TEXTURE_PREFIX = "texture";

static bool startWith(const char* &start, const char* prefix, size_t prefixLength)
{
	if(std::strncmp(start, prefix, prefixLength) == 0)
	{
		start += prefixLength;
		return true;
	}
	
	return false;
}

void Mesh::setProgram(uint32_t program)
{
	uint32_t oldProgram = getProgram();
	if(oldProgram == program)
		return;
	
	Object::setProgram(program);
	
	if(!texcoords.empty())
		assert(glGetAttribLocation(program, "texcoord") == Shader::ATTRIBUTE_VEC_TEXCOORD);
	
	if(!colors.empty())
		assert(glGetAttribLocation(program, "color") == Shader::ATTRIBUTE_VEC_COLOR);
	
	// if normal is specified but not used in program, assertion fails.
	int32_t normalLocation = glGetAttribLocation(program, "normal");
//	if(!normals.empty())
//		assert(normalLocation == Shader::ATTRIBUTE_VEC_NORMAL);
	assert(normalLocation == Shader::ATTRIBUTE_VEC_NORMAL || normalLocation == Shader::INVALID_LOCATION);
	
	if(!instances1.empty())
		assert(glGetAttribLocation(program, "instance") == Shader::ATTRIBUTE_MAT_INSTANCE);
	else if(!instances0.empty())
		assert(glGetAttribLocation(program, "instance") == Shader::ATTRIBUTE_VEC_INSTANCE);
	else
		assert(glGetUniformLocation(program, "model") == Shader::UNIFORM_MAT_MODEL);
	
	viewProjectionLocation = glGetUniformLocation(program, "viewProjection");
	assert(viewProjectionLocation == Shader::UNIFORM_MAT_VIEW_PROJECTION ||
			viewProjectionLocation != Shader::INVALID_LOCATION);
	
	if(oldProgram != 0)
	{
		textureMap.clear();
		textures.clear();
	}

	std::vector<Program::Variable> variables = Program::getActiveVariables(program, GL_ACTIVE_UNIFORMS);
	uint32_t textureUnit = 0;
	
	Program::use(program);
	for(const Program::Variable& variable: variables)
	{
		// type+index -> location
		if(variable.type < GL_SAMPLER_1D || variable.type > GL_SAMPLER_CUBE)
			continue;
		
		// convention, texture variable name must begin with "texture".
		const size_t length = 7;  // std::strlen(TEXTURE_PREFIX)
		const char* str = variable.name.c_str();
		assert(std::strncmp(str, TEXTURE_PREFIX, length) == 0);
		size_t end = variable.name.size();
		while(std::isdigit(variable.name[end - 1]))
			--end;
		
		Texture::Type type;
		const char* start = str + length;
		if(length == end)
			type = Texture::Type::NONE;
		else if(startWith(start, "Diffuse", 7))
			type = Texture::Type::DIFFUSE;
		else if(startWith(start, "Specular", 8))
			type = Texture::Type::SPECULAR;
		else if(startWith(start, "Shininess", 9))
			type = Texture::Type::SHININESS;
		else if(startWith(start, "Normal", 6))
			type = Texture::Type::NORMAL;
		else if(startWith(start, "Height", 6))
			type = Texture::Type::HEIGHT;
		else
		{
			slog.w(TAG, "name=%s, texture type %s is not defined.", str, start);
			continue;
		}

		uint32_t index;
		if(length < end)
			index = static_cast<uint32_t>(std::atoi(start));
		else  // no trailing digits
			index = 0U;
		
		uint32_t key = Texture::getKey(type, index);
		glUniform1i(variable.location, textureUnit);
		textureMap[key] = textureUnit;
		slog.d(TAG, "%s key=0x%08X, occupies texture unit #%d", str, key, textureUnit);
		++textureUnit;
	}
}

bool Mesh::hasTextureUnit(Texture::Type type, uint32_t index) const
{
	uint32_t key = Texture::getKey(type, index);
	return textureMap.find(key) != textureMap.end();
}

void Mesh::setTexture(const Texture& texture, uint32_t index)
{
	assert(getProgram() != Program::NULL_PROGRAM);  // setProgram() must be called ahead.
/*
	const uint32_t program = getProgram();
	assert(program != Program::NULL_PROGRAM);

	std::string textureName;
	switch(texture->getType())
	{
	case Texture::Type::DIFFUSE:   textureName = "Diffuse";   break;
	case Texture::Type::SPECULAR:  textureName = "Specular";  break;
	case Texture::Type::NORMAL:    textureName = "Normal";    break;
	case Texture::Type::HEIGHT:    textureName = "Height";    break;
	case Texture::Type::NONE:      textureName = "";          break;
	}
	textureName = TEXTURE_PREFIX + textureName + std::to_string(index);
	uint32_t location = glGetUniformLocation(program, textureName.c_str());
*/
	uint32_t key = texture.getKey(index);
	assert(textureMap.find(key) != textureMap.end());
	uint32_t textureUnit = textureMap.at(key);
	textures[textureUnit] = &texture;  // or textures.insert_or_assign(textureUnit, &texture); C++17
}

bool Mesh::hasUniform(int32_t location) const
{
	return uniformBlock.hasUniform(location);
}

void Mesh::setUniform(int32_t location, Type type, void* data)
{
	uniformMutex.lock();
	
	Uniform* uniform;
	if(uniformBlock.hasUniform(location, uniform)) [[likely]]
	{
		assert(uniform->type == type);
		uniformBlock.updateUniform(*uniform, data);
	}
	else
		uniformBlock.appendUniform(location, type, data);
	
	uniformMutex.unlock();
}

void Mesh::render(const mat4f& viewProjection) const
{
//	Object::render(viewProjection);
	uint32_t program = getProgram();
	assert(program != 0);  // make sure that setProgram() has been called
	Program::use(program);
	
	const int32_t instanceCount = instances0.size() > 0? instances0.size(): instances1.size();
	if(instanceCount == 0)
		Program::setUniform(Shader::UNIFORM_MAT_MODEL, getTransform());
	if(viewProjectionLocation == Shader::UNIFORM_MAT_VIEW_PROJECTION)
		Program::setUniform(Shader::UNIFORM_MAT_VIEW_PROJECTION, viewProjection);
	
	uniformMutex.lock();
	uniformBlock.feedUniforms();
	uniformMutex.unlock();
	
	// textures binding
	for(const std::pair<const uint32_t, const Texture*>& pair: textures)
	{
		const uint32_t& textureUnit = pair.first;
		const Texture* const &texture = pair.second;
		assert(texture != nullptr);
		texture->bind(textureUnit);
//		slog.d(TAG, "bind texture %d to unit #%d", texture->getName(), textureUnit);
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
	
//	Program::setUniform(Shader::UNIFORM_MAT_VIEW_PROJECTION, viewProjection);
	GLenum mode = GL::getPrimitive(primitive);
	// TODO: primitive restart
//	slog.i(TAG, "vertices.size()=%zu, normals.size()=%zu, indices.size()=%zu, mode=0x%X", vertices.size(), normals.size(), indices.size(), mode);
	assert(vao != 0);  // make sure that prepare() is called
	glBindVertexArray(vao);
	const int32_t vertexCount = getVertexSize();
	const int32_t indexCount = indices.size();
	if(instanceCount > 0)  // instance rendering
	{
		if(indexCount > 0)
			glDrawElementsInstanced(mode, indexCount, GL_UNSIGNED_INT, nullptr, instanceCount);
		else
			glDrawArraysInstanced(mode, 0, vertexCount, instanceCount);
	}
	else
	{
		if(indexCount > 0)
			glDrawElements(mode, indexCount, GL_UNSIGNED_INT, nullptr);
		else
			glDrawArrays(mode, 0, vertexCount);
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
			"polygon",
			"lines_adjacency", "line_strip_adjacency", "triangles_adjacency", "triangle_strip_adjacency",
			"patches"
		};
		const char* mode = PRIMITIVE_STRINGS[underlying_cast<Primitive>(primitive)];
		if(instanceCount > 0)
		{
			if(indexCount > 0)
				slog.d(TAG, "glDrawElementsInstanced(%s), size=%zu, instance=%zu", mode, indexCount, instanceCount);
			else
				slog.d(TAG, "glDrawArraysInstanced(%s), size=%zu, instance=%zu", mode, vertexCount, instanceCount);
		}
		else
		{
			if(indexCount > 0)
				slog.d(TAG, "glDrawElements(%s), size=%zu", mode, indexCount);
			else
				slog.d(TAG, "glDrawArrays(%s), size=%zu", mode, vertexCount);
		}

		printed = true;
	}
#endif
	glBindVertexArray(0);
}

Mesh::Builder::Builder(const std::vector<vec4f>& vertices):
		vertices(vertices),
		positions(0)
{
	assert(!vertices.empty());
}

Mesh::Builder::Builder(const std::vector<vec3f>& positions):
		vertices(0),
		positions(positions)
{
	assert(!positions.empty());
}

// Either vertices or positions is empty.
size_t Mesh::Builder::getVertexSize() const
{
	size_t size = positions.size();  // mostly vec3 position
	if(size > 0)
		return size;
	return vertices.size();
}

Mesh::Builder& Mesh::Builder::setTexcoord(const std::vector<vec2f>& texcoords)
{
	assert(getVertexSize() == texcoords.size());
	this->texcoords = texcoords;
	return *this;
}

Mesh::Builder& Mesh::Builder::setColor(const std::vector<vec3f>& colors)
{
	assert(getVertexSize() == colors.size());
	this->colors = std::move(colors);
	return *this;
}

Mesh::Builder& Mesh::Builder::setNormal(const std::vector<vec3f>& normals)
{
	assert(getVertexSize() == normals.size());
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

Mesh::Builder::Builder(std::vector<vec4f>&& vertices):
		vertices(std::move(vertices)),
		positions(0)
{
	assert(!this->vertices.empty());
}

Mesh::Builder::Builder(std::vector<vec3f>&& positions):
		vertices(0),
		positions(std::move(positions))
{
	assert(!this->positions.empty());
}

Mesh::Builder& Mesh::Builder::setTexcoord(std::vector<vec2f>&& texcoords)
{
	assert(getVertexSize() == texcoords.size());
	this->texcoords = std::move(texcoords);
	return *this;
}

Mesh::Builder& Mesh::Builder::setColor(std::vector<vec3f>&& colors)
{
	assert(getVertexSize() == colors.size());
	this->colors = std::move(colors);
	return *this;
}

Mesh::Builder& Mesh::Builder::setNormal(std::vector<vec3f>&& normals)
{
	assert(getVertexSize() == normals.size());
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
	uint32_t vertexCount = getVertexSize();
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

	const uint32_t size = positions.size();
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
			vertex.position = positions[i];
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
			vertex.position = positions[i];
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

	createIndex(positions , indexMap);
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
	
	Group::dropIndex(positions, indices);
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
	mesh->positions= std::move(positions);
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
