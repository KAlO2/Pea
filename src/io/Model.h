#ifndef PEA_IO_MODEL_H_
#define PEA_IO_MODEL_H_

#include <string>
#include <memory>
#include <vector>
#include <map>

#include "math/vec2.h"
#include "math/vec3.h"
#include "io/Group.h"

namespace pea {

class Mesh;

/**
 * model data in memory. It supports ngon.
 */
class Model
{
public:
	enum class TriangulationMethod: uint32_t
	{
		BEAUTY = 0,  // Delaunay triangulation maximizes the minimum angle, not the edge-length of the triangles.
		FIXED,
		ALTERNATE,
		SHORT_EDGE,
	};
	
	class Edge
	{
	public:
		uint32_t vertex0;
		uint32_t vertex1;
		
		friend bool operator <(const Edge& e0, const Edge& e1)
		{
			return std::tie(e0.vertex0, e0.vertex1) < std::tie(e1.vertex0, e1.vertex1);
		}
	};
private:
	std::vector<vec3f> vertices;
	std::vector<vec2f> texcoords;
	std::vector<vec3f> normals;
	
	std::vector<uint32_t> triangleIndices;  // each three indices forms a triangle
	std::vector<uint32_t> quadrilateralIndices;
	std::vector<uint32_t> polygonIndices;
	std::vector<uint32_t> polygonVertexSizes;
	std::vector<Edge> lines;  // each two indices forms a line.
	std::vector<Edge> seamEdges;  // UV mapping, one vertex maps to two texcoords
	std::vector<Edge> sharpEdges;  // nomal, one vertex maps to two normals
	
	std::map<std::string, Group> groups;
	
	
private:
	friend class Model_OBJ;
	
//	bool isConnected(uint32_t vertex0, uint32_t vertex1) const;
	uint32_t findEdge(uint32_t vertex0, uint32_t vertex1, uint32_t faces[2]) const;
	
//	std::vector<uint32_t> findNeighbourVertex(
	void traverseFace(void (*function)(uint32_t faceIndex, const uint32_t* array, uint32_t length, void* data), void* data) const;
	
//	uint32_t* getVertexIndexOfFace(size_t index, uint32_t& size);
	
public:
	Model();
	virtual ~Model();
	
	// vertex operations
	const std::vector<vec3f>& getVertexData() const;
	
	uint32_t addVertex(const vec3f& vertex);
	
	uint32_t addVertex(const std::vector<vec3f>& vertexGroup);
	
	void removeVertex(uint32_t index);
	
	/**
	 * @param[in] vertex
	 * @param[out] index
	 * @return vertex exist or not.
	 */
	bool findVertex(const vec3f& vertex, uint32_t* index);

	/**
	 * @param[in] faceNormals face normalized normal computed by #computeFaceNormal().
	 */
	std::vector<vec3f> computeVertexNormal() const;
//	void selectMore(Group& group);
	
	
	// edge operations
	std::vector<uint32_t> getEdgeIndex() const;
	
	// face operations
	size_t getFaceSize() const;
	size_t getTriangulatedFaceSize() const;
	/**
	 * @param[in] index Face index
	 * @param[out] size Vertex count of the face 
	 * @return vertex indices of face #index.
	 */
	const uint32_t* getVertexIndexOfFace(size_t index, uint32_t& size) const;
	
	void flipFaceDirection(size_t index);
	
	std::vector<uint32_t> getTriangulatedIndex() const;
	
	/**
	 * @param[in] index vertex index
	 * @param[in] length polygon vertex number
	 */
	void addFace(const uint32_t* index, uint32_t length);
	
	/**
	 * @param[in] index face index
	 */
	void removeFace(uint32_t index);
	
	std::vector<uint32_t> selectFaceForVertex(uint32_t index) const;
	
	/**
	 * face point order: triangles, quadrilaterals, polygons.
	 */
	std::vector<vec3f> computeFacePoint() const;
	
	std::vector<float> computeFaceArea() const;
	
	/**
	 * Generates facet normals for a mesh (by taking the cross product of the two vectors
	 * derived from the sides of each triangle). Assumes a counter-clockwise winding.
	 */
	std::vector<vec3f> computeFaceNormal() const;
	
	void triangulate(TriangulationMethod method);
	
	bool areFacesTriangulated() const;
	
	// group operations
	std::vector<std::string> getGroupNames() const;
	
	/**
	 * add a new group
	 */
	void addGroup(const std::string& name, const Group& group);
	void addGroup(const std::string& name, Group&& group);
	
	/**
	 * append data to the specified group.
	 */
	void appendGroup(const std::string& name, const Group& group);
	
	void removeGroup(const std::string& name);
	
	bool findGroup(const std::string& name, Group& group) const;
	
	Model subdivide() const;
	
//	void subdivideTriangleIntoQuad();
	
	// io, export to mesh for rendering, save as Model_OBJ format for archiving.
//	std::unique_ptr<Mesh> extract() const;
	
//	void removeDoubles();
	
	

};

}  // namespace pea
#endif  // PEA_IO_MODEL_H_
