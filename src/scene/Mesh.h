#ifndef PEA_SCENE_MESH_H_
#define PEA_SCENE_MESH_H_

#include "geometry/BoundingBox.h"
#include "geometry/Primitive.h"
#include "io/Type.h"
#include "math/vec2.h"
#include "math/vec3.h"
#include "math/mat4.h"
#include "opengl/Texture.h"
#include "opengl/UniformBlock.h"
#include "scene/Bone.h"
#include "scene/Object.h"

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace pea {

class Material;
class Light;

enum class RenderMode
{
	VERTEX,   // render with only positions
	FLAT,     // render with facet normals
	SMOOTH,   // render with vertex normals
	COLOR,    // render with colors
	TEXTURE,  // render with texture coordinates
	MATERIAL, // render with materials
};

// our meshes are triangulated
// http://danenglesson.com/images/portfolio/MoA/halfedge.pdf
class Mesh: public Object
{
public:
	friend class Model_OBJ;
	static constexpr int32_t VBO_COUNT = 11;
	
private:
	std::string name;

	std::vector<vec4f> vertices;
	std::vector<vec3f> positions;
	std::vector<vec2f> texcoords;
	std::vector<vec3f> colors;
	
	std::vector<vec3f> normals;
	std::vector<vec3f> tangents;
	std::vector<vec3f> bitangents;
	
	std::vector<uint32_t> indices;
//	std::unordered_map<std::string, uint64_t> groups;  // <group name, min max pair>, can be empty
	
	// instances0 takes higher priorty than instances1 on instance rendering.
	std::vector<vec4f> instances0;  // vec4 = vec3 position + float scale;
	std::vector<mat4f> instances1;  // mat4 transform;
	
	// key is texture type|index, value is texture unit
	std::unordered_map<uint32_t, uint32_t> textureMap;
	
	// key is texture unit
	std::unordered_map<uint32_t, const Texture*> textures;
//	std::string materialName;  // empty for no material
	Material* material;
	std::vector<Light*> lights;
//	std::vector<vec2i>         edges;

	std::vector<Bone>  bones;
//	std::vector<Color> colors;  // color format definition

	std::vector<vec3f> faceNormals;
//	std::vector<TextureIndex> face_textures;
	
	// vertex attributes, using constexpr here, enum introduces a type
/*	static constexpr uint32_t VA_POSITION = 0;
	static constexpr uint32_t VA_TEXCOORD = 1;
	static constexpr uint32_t VA_COLOR    = 2;
	static constexpr uint32_t VA_NORMAL   = 3;
	static constexpr uint32_t VA_TANGENT  = 4;
	static constexpr uint32_t VA_BITANGENT= 5;
//	static constexpr uint32_t VA_INDEX    = 6;
	static constexpr uint32_t VA_INSTANCE = 7;
	static constexpr uint32_t VA_COUNT    = 8;
*/
	uint32_t vao;
	uint32_t vbo[VBO_COUNT];
	Primitive primitive;
	
	int32_t viewProjectionLocation;
	
	UniformBlock uniformBlock;
	mutable std::mutex uniformMutex;
	
//	std::unordered_map<std::string, int32_t> attributes;
//	std::map<std::string, int32_t> attributes;
private:

	inline bool hasFaceNormal() const;
	
	void remapIndex(std::vector<uint32_t>&& indexMap, const uint32_t& size);
	
	bool verifyBone() const;

public:
	class Builder;
	friend Builder;
	
	Mesh();
	virtual ~Mesh();

	Mesh(const Mesh& mesh);
	Mesh& operator =(const Mesh& mesh);
	
	Mesh(Mesh&& mesh);
	Mesh& operator =(Mesh&& mesh);
	
	void setName(const std::string& name);
	const std::string& getName() const;
	
	void applyTransform();
	
	bool hasNormal() const;

	
	void computeTangentBasis();
	
	size_t getVertexSize() const;
	
	size_t getIndexSize() const;
	
	/**
	 * calculate the AABB of the mesh
	 * note that the AABB is in the local space, not the world space
	 */
	BoundingBox getBoundingBox() const;

	/**
	 * Reverse the triangle winding for all the triangles in this mesh.  Default winding
	 * is counter-clockwise. It also changes the direction of the normals.
	 */
	void reverseWinding();
	
	void setMaterial(Material* material);
	const Material* getMaterial() const;

	void addLight(Light* light);
	void removeLight(const Light* light);
	
	bool hasTextureUnit(Texture::Type type, uint32_t index) const;
	
	/**
	 * @param[in] texture The new texture to be replaced.
	 * @param[in] index textureUnit 0 for GL_TEXTURE0.
	 */
	void setTexture(const Texture& texture, uint32_t index);
	
	/**
	 * Allocate VAO, VBO and specify the primitive to draw.
	 * @param[in] primitive Draw call primitive.
	 */
	virtual void prepare(Primitive primitive = Primitive::TRIANGLES);
	
	/**
	 * must call after preprare(), or you will get zero.
	 */
	uint32_t getVertexBufferObject(uint32_t attributeIndex) const;
	
	/**
	 * Store object positions, normals and/or indices in graphic card buffers
	 */
	void upload() const;
	
	// TODO: 
	void upload(uint32_t vboFlag[VBO_COUNT]) const;
	
	/**
	 * @param[in] location Vertex attribute location.
	 * @return VBO, generated by glGenBuffers
	 */
	uint32_t getVertexBufferObject(int32_t location) const;
	
	// non virtual function
	void setProgram(uint32_t program);
	
	bool hasUniform(int32_t location) const;
	
	/**
	 * call this method after #setProgram()
	 */
	void setUniform(int32_t location, Type type, void* data);
	
//	void updateUniform(Uniform& uniform, void* data);
	
	/**
	 * @param[in] mode Specifies what kind of primitives to render. Symbolic constants GL_POINTS,
	 *                 GL_LINE_STRIP, GL_TRIANGLE_STRIP.
	 */
	void render(const mat4f& viewProjection) const override;

};

inline bool Mesh::hasFaceNormal() const { return !faceNormals.empty(); }
inline bool Mesh::hasNormal() const     { return !normals.empty();     }

inline void Mesh::setName(const std::string& name) { this->name = name; }
inline const std::string& Mesh::getName() const    { return name;       }

inline void Mesh::setMaterial(Material* material) { this->material = material; }
inline const Material* Mesh::getMaterial() const { return material; }


class Mesh::Builder
{
private:
	std::vector<vec4f> vertices;
	std::vector<vec3f> positions;
	std::vector<vec3f> colors;
	std::vector<vec3f> normals;
	std::vector<vec2f> texcoords;

	std::vector<vec3f> tangents;
	std::vector<vec3f> bitangents;
	
	std::vector<vec4f> instances0;
	std::vector<mat4f> instances1;
	std::vector<uint32_t> indices;
	
private:
	void verifyIndex(const std::vector<uint32_t>& indices) const;
	
	size_t getVertexSize() const;
	
public:
	explicit Builder(const std::vector<vec4f>& vertices);
	explicit Builder(const std::vector<vec3f>& positions);
	
	Builder& setColor(const std::vector<vec3f>& colors);
	
	Builder& setNormal(const std::vector<vec3f>& normals);
	
	Builder& setTexcoord(const std::vector<vec2f>& texcoords);
	
	Builder& setInstance(const std::vector<vec4f>& instances);
	Builder& setInstance(const std::vector<mat4f>& instances);
	
	Builder& setIndex(const std::vector<uint32_t>& indices);
	
	// rvalue version
	explicit Builder(std::vector<vec4f>&& vertices);
	explicit Builder(std::vector<vec3f>&& positions);
	
	Builder& setColor(std::vector<vec3f>&& colors);
	
	Builder& setNormal(std::vector<vec3f>&& normals);
	
	Builder& setTexcoord(std::vector<vec2f>&& texcoords);
	
	Builder& setInstance(std::vector<vec4f>&& instances);
	Builder& setInstance(std::vector<mat4f>&& instances);
	
	Builder& setIndex(std::vector<uint32_t>&& indices);
	
	bool isIndexed() const;
	
	/**
	 * shrink v/vt/vn data for indexing, remove duplicate vertex data.
	 */
	Builder& shrinkToIndex();
	
	/**
	 * the opposite operation of #shrinkToIndex(). 
	 * remove index data by updating vertex data. Two same indices will expand to two vertices.
	 */
	Builder& removeIndex();
	
	std::unique_ptr<Mesh> build();
};

}  // namespace pea
#endif  // PEA_SCENE_MESH_H_
