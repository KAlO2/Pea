#ifndef PEA_SCENE_MESH_H_
#define PEA_SCENE_MESH_H_

#include "geometry/BoundingBox.h"
#include "geometry/Primitive.h"
#include "math/vec2.h"
#include "math/vec3.h"
#include "math/mat4.h"
#include "opengl/Texture.h"
#include "scene/Bone.h"
#include "scene/Object.h"

//#include <list>
#include <memory>
#include <string>
//#include <unordered_map>
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
class Mesh: public Object
{
public:
	friend class Model_OBJ;

private:
	std::string name;

	std::vector<vec3f> vertices;
	std::vector<vec2f> texcoords;
	std::vector<vec3f> colors;
	
	std::vector<vec3f> normals;
	std::vector<vec3f> tangents;
	std::vector<vec3f> bitangents;
	
	std::vector<uint32_t> indices;
	
	// instances0 takes higher priorty than instances1 on instance rendering.
	std::vector<vec4f> instances0;  // vec4 = vec3 position + float scale;
	std::vector<mat4f> instances1;  // mat4 transform;
	
	std::vector<std::shared_ptr<Texture>> textures;

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
	uint32_t vbo[11];
	Primitive primitive;
	
	int32_t viewPositionLocation;
	vec3f viewPosition;
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
	
	bool hasIndex() const;
	
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
	
	void setViewPosition(const vec3f& position);
	const vec3f& getViewPosition() const;
	
	void setTexture(const std::vector<std::shared_ptr<Texture>>& textures);
	void setTexture(std::vector<std::shared_ptr<Texture>>&& textures);
//	const std::unordered_map<std::string, std::shared_ptr<Texture>>& getTexture() const;

	void prepare(Primitive primitive = Primitive::TRIANGLES);
	
	/**
	 * Store object positions, normals and/or indices in graphic card buffers
	 */
	void upload() const;
	
	// not virtual function
	void setProgram(uint32_t program);
	
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

inline void Mesh::setViewPosition(const vec3f& position) { viewPosition = position; }
inline const vec3f& Mesh::getViewPosition() const { return viewPosition; }

inline void Mesh::setTexture(const std::vector<std::shared_ptr<Texture>>& textures)
{
	this->textures = textures;
}

inline void Mesh::setTexture(std::vector<std::shared_ptr<Texture>>&& textures)
{
	this->textures = std::move(textures);
}

class Mesh::Builder
{
private:
	std::vector<vec3f> vertices;
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
	
public:
	explicit Builder(const std::vector<vec3f>& vertices);
	
	Builder& setColor(const std::vector<vec3f>& colors);
	
	Builder& setNormal(const std::vector<vec3f>& normals);
	
	Builder& setTexcoord(const std::vector<vec2f>& texcoords);
	
	Builder& setInstance(const std::vector<vec4f>& instances);
	Builder& setInstance(const std::vector<mat4f>& instances);
	
	Builder& setIndex(const std::vector<uint32_t>& indices);
	
	// rvalue version
	explicit Builder(std::vector<vec3f>&& vertices);
	
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
