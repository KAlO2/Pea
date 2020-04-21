#include "io/Model.h"

#include <algorithm>
#include <numeric>  // std::accumulate is in <numeric>, not <algorithm>
#include <cinttypes>
#include <set>
#include <unordered_map>

#include "scene/Mesh.h"
#include "util/Log.h"


using namespace pea;

static const char* TAG = "Model";

static uint64_t makeEdge(uint32_t vertex0, uint32_t vertex1)
{
	assert(vertex0 != vertex1);
	if(vertex0 < vertex1)
		return static_cast<uint64_t>(vertex0) | (static_cast<uint64_t>(vertex1) << 32);
	else
		return static_cast<uint64_t>(vertex1) | (static_cast<uint64_t>(vertex0) << 32);
}

Model::Model()
{
	// TODO Auto-generated constructor stub
	
}

Model::~Model()
{
	// TODO Auto-generated destructor stub
}

const std::vector<vec3f>& Model::getVertexData() const
{
	return vertices;
}

uint32_t Model::addVertex(const vec3f& vertex)
{
	uint32_t size = vertices.size();
	vertices.push_back(vertex);
//	std::cout << "add vertex " << vertex << '\n';
	return size;
}

uint32_t Model::addVertex(const std::vector<vec3f>& vertexGroup)
{
	uint32_t startSize = vertices.size();
	size_t vertexCount = startSize + vertexGroup.size();
	vertices.reserve(vertexCount);
	
	for(const vec3f& vertex: vertexGroup)
		vertices.push_back(vertex);
/*
	for(uint32_t i = 0; i < vertices.size(); ++i)
		std::cout << "vertex #" << i << " position: " << vertices[i] << '\n';
	std::cout << "add many vertices\n";
*/
	return startSize;
}

void Model::removeVertex(uint32_t index)
{
	if(index >= vertices.size())
		return;
	
	// remove connected faces
	std::vector<uint32_t> faceIndices = selectFaceForVertex(index);
	// remove element from vector container, inefficient.
	for(const uint32_t& faceIndex: faceIndices)
		removeFace(faceIndex);
	
	// TODO: lines
	
	// decrease index
	for(uint32_t& i: triangleIndices)
		if(i > index)
			--i;
	for(uint32_t& i: quadrilateralIndices)
		if(i > index)
			--i;
	for(uint32_t& i: polygonIndices)
		if(i > index)
			--i;
	
	// remove vertex
	auto it = vertices.begin() + index;
	vertices.erase(it, it + 1);
}

bool Model::findVertex(const vec3f& vertex, uint32_t* index)
{
	const uint32_t size = vertices.size();
	for(uint32_t i = 0; i < size; ++i)
	{
		if(vertices[i] == vertex)
		{
			if(index)
				*index = i;
			return true;
		}
	}
	
	return false;
}

std::vector<vec3f> Model::computeVertexNormal() const
{
	std::vector<vec3f> faceNormals = computeFaceNormal();
	std::vector<float> faceAreas = computeFaceArea();
/*	for(uint32_t i = 0; i < vertices.size(); ++i)
		std::cout << "vertex #" << i << " position: " << vertices[i] << '\n';
	for(uint32_t i = 0; i < quadrilaterals.size(); i += 4)
		std::cout << "face #" << i / 4 << " index:" << 
		quadrilaterals[i] << ' ' << quadrilaterals[i + 1] << ' ' << quadrilaterals[i + 2] << ' ' << quadrilaterals[i + 3] << '\n';
	for(uint32_t i = 0; i < faceNormals.size(); ++i)
		std::cout << "face #" << i << " normal: " << faceNormals[i] << ", area=" << faceAreas[i] << '\n';
*/
	const size_t vertexSize = vertices.size();
	std::vector<vec3f> vertexNormals(vertexSize, vec3f(0, 0, 0));
	
	auto addWeight = [&](uint32_t vertexIndex, uint32_t faceIndex)
	{
		vertexNormals[vertexIndex] += faceAreas[faceIndex] * faceNormals[faceIndex];
	};
	
	const uint32_t triangleIndexSize = triangleIndices.size();
	const uint32_t quadrilateralIndexSize = quadrilateralIndices.size();
	const uint32_t polygonIndexSize = polygonVertexSizes.size();
	
	uint32_t faceStart = 0;
	for(uint32_t i = 0; i < triangleIndexSize; ++i)
		addWeight(triangleIndices[i], i / 3);
	
	faceStart += triangleIndexSize / 3;
	for(uint32_t i = 0; i < quadrilateralIndexSize; ++i)
		addWeight(quadrilateralIndices[i], faceStart + i / 4);
	
	faceStart += quadrilateralIndexSize >> 2;
	for(uint32_t i = 0; i < polygonIndexSize; ++i)
		addWeight(polygonIndices[i], faceStart + i);
//	for(uint32_t i = 0; i < vertexNormals.size(); ++i)
//		std::cout << "vertex #" << i << " normal sum: " << vertexNormals[i] << '\n';
	for(vec3f& normal: vertexNormals)
		normal.normalize();
	
	return vertexNormals;
}

std::vector<uint32_t> Model::getEdgeIndex() const
{
	std::set<uint64_t> edgeSet;
//	size_t edgeBudget = triangleIndices.size() * 3 / 2 + quadrilateralIndices.size() * 2 + polygonVertexSizeIndices.size() * 5 / 2;
//	edgeSet.reserve(edgeBudget);
	auto addLines = [](uint32_t faceIndex, const uint32_t* array, uint32_t length, void* data)
	{
		std::ignore = faceIndex;
		for(uint32_t index0 = 0; index0 < length; ++index0)
		{
			uint32_t index1 = (index0 + 1) % length;
			uint32_t vertex0 = array[index0];
			uint32_t vertex1 = array[index1];
			
			uint64_t edge = makeEdge(vertex0, vertex1);
			static_cast<std::set<uint64_t>*>(data)->insert(edge);
		}
	};
	
	traverseFace(addLines, &edgeSet);
	
	std::vector<uint32_t> edges;
	edges.reserve(edgeSet.size());
	for(const uint64_t& pair: edgeSet)
	{
		const uint32_t* vertex = reinterpret_cast<const uint32_t*>(&pair);
		edges.push_back(vertex[0]);
		edges.push_back(vertex[1]);
	}
	return edges;
}

size_t Model::getFaceSize() const
{
	assert(triangleIndices.size() % 3 == 0);
	assert(quadrilateralIndices.size() % 4 == 0);
	
	return triangleIndices.size() / 3 + quadrilateralIndices.size() / 4 + polygonVertexSizes.size();
}

size_t Model::getTriangulatedFaceSize() const
{
	// polygon with n vertices => (n - 2) triangles
	size_t sum = std::accumulate(polygonVertexSizes.begin(), polygonVertexSizes.end(), 0U);
	sum -= 2 * polygonVertexSizes.size();
	return triangleIndices.size() / 3 + quadrilateralIndices.size() / 2 + sum;
}

const uint32_t* Model::getVertexIndexOfFace(size_t index, uint32_t& size) const
{
	size_t triangleSize = triangleIndices.size() / 3;
	if(index < triangleSize)
	{
		size = 3;
		return triangleIndices.data() + index * 3;
	}
	
	index -= triangleSize;
	size_t quadrilateralSize = quadrilateralIndices.size() >> 2;
	if(index < quadrilateralSize)
	{
		size = 4;
		return quadrilateralIndices.data() + (index << 2);
	}
	
	index -= quadrilateralSize;
	assert(index < polygonVertexSizes.size());
	size_t sum = std::accumulate(polygonVertexSizes.begin(), polygonVertexSizes.begin() + index, 0U);
	size = polygonVertexSizes[index];
	return polygonIndices.data() + sum;
}

void Model::flipFaceDirection(size_t index)
{
	uint32_t size;
	const uint32_t* start = getVertexIndexOfFace(index, size);
	uint32_t* data = const_cast<uint32_t*>(start);
	for(size_t i = 1, j = size - 1; i < j; ++i, --j)
		std::swap(data[i], data[j]);
}

std::vector<uint32_t> Model::getTriangulatedIndex() const
{
	// polygon with n vertices => 3 * (n - 2) triangles
	size_t face3Start = 0;
	size_t face4Start = triangleIndices.size();
	size_t faceNStart = face4Start + (quadrilateralIndices.size() >> 1u) * 3;
	
	uint32_t index = std::accumulate(polygonVertexSizes.begin(), polygonVertexSizes.end(), 0);
	index = 3 * (index - polygonVertexSizes.size() * 2);
	size_t size = faceNStart + index;
	
	std::vector<uint32_t> faces(size);
	std::copy(triangleIndices.begin(), triangleIndices.end(), faces.begin());
//	for(uint32_t index: triangleIndices)
//		faces.push_back(index);
	
	std::vector<uint32_t> face4 = quadrilateralsToTriangles(quadrilateralIndices.data(), quadrilateralIndices.size());
	std::copy(face4.begin(), face4.end(), faces.begin() + face4Start);
	
	std::vector<uint32_t> faceN = polygonsToTriangles(polygonIndices.data(), polygonVertexSizes.data(), polygonVertexSizes.size());
	std::copy(faceN.begin(), faceN.end(), faces.begin() + faceNStart);
	
//	assert(faces.size() == size);  // to make sure that we get the right size
	return faces;
}

void Model::addFace(const uint32_t* index, uint32_t length)
{
	assert(index != nullptr && length >= 3);
	if(length == 3)
	{
		triangleIndices.push_back(index[0]);
		triangleIndices.push_back(index[1]);
		triangleIndices.push_back(index[2]);
	}
	else if(length == 4)
	{
		quadrilateralIndices.push_back(index[0]);
		quadrilateralIndices.push_back(index[1]);
		quadrilateralIndices.push_back(index[2]);
		quadrilateralIndices.push_back(index[3]);
	}
	else
	{
		for(uint32_t i = 0; i < length; ++i)
			polygonIndices.push_back(index[i]);
		polygonVertexSizes.push_back(length);
	}
}

void Model::removeFace(uint32_t index)
{
	uint32_t faceStart = 0, faceStop = triangleIndices.size() / 3;
	if(index < faceStop)
	{
		auto it = triangleIndices.begin() + index * 3;
		triangleIndices.erase(it, it + 3);
		return;
	}
	
	faceStart = faceStop;
	faceStop += quadrilateralIndices.size() >> 2;
	if(index < faceStop)
	{
		auto it = quadrilateralIndices.begin() + (index - faceStart) * 4;
		quadrilateralIndices.erase(it, it + 4);
		return;
	}
	
	faceStart = faceStop;
	faceStop += polygonVertexSizes.size();
	if(index < faceStop)
	{
		auto begin = polygonVertexSizes.begin();
		auto end = begin + (index - faceStart);
		uint32_t count = std::accumulate(begin, end, 0u);
		auto it = polygonIndices.begin() + count;
		polygonIndices.erase(it, it + polygonVertexSizes[index - faceStart]);
		polygonVertexSizes.erase(end, end + 1);
	}
}

std::vector<uint32_t> Model::selectFaceForVertex(uint32_t index) const
{
/*
	auto checkFace = [index](uint32_t faceIndex, const uint32_t* array, uint32_t length, void* data)
	{
		std::vector<uint32_t> faces = *reinterpret_cast<std::vector<uint32_t>*>(data);
		for(uint32_t i = 0; i < length; ++i)
		{
			if(array[i] == index)
				faces.push_back(faceIndex);
		}
	};
	
	std::vector<uint32_t> faces;
	traverseFace(checkFace, faces);
*/
	uint32_t faceIndex = 0;
	std::vector<uint32_t> faces;
	
	const uint32_t* start = triangleIndices.data();
	const uint32_t* end = start + triangleIndices.size();
	for(; start < end; start += 3, ++faceIndex)
		if(start[0] == index || start[1] == index || start[2] == index)
			faces.push_back(faceIndex);
	
	start = quadrilateralIndices.data();
	end = start + quadrilateralIndices.size();
	for(; start < end; start += 4, ++faceIndex)
		if(start[0] == index || start[1] == index || start[2] == index || start[3] == index)
			faces.push_back(faceIndex);
	
	start = polygonIndices.data();
	end = start + polygonIndices.size();
//	const uint32_t* size_end = polygonVertexSizes.data() + polygonVertexSizes.size();
	for(const uint32_t* size = polygonVertexSizes.data(); start < end; start += *size, ++size, ++faceIndex)
		for(uint32_t i = 0; i < *size; ++i)
		{
			if(start[i] == index)
			{
				faces.push_back(faceIndex);
				break;
			}
		}
	return faces;
}

void Model::traverseFace(void (*function)(uint32_t faceIndex, const uint32_t* array, uint32_t length, void* data), void* data) const
{
	uint32_t faceIndex = 0;
	
	const uint32_t* start = triangleIndices.data();
	const uint32_t* end = start + triangleIndices.size();
	for(; start < end; start += 3, ++faceIndex)
		function(faceIndex, start, 3, data);
	
	start = quadrilateralIndices.data();
	end = start + quadrilateralIndices.size();
	for(; start < end; start += 4, ++faceIndex)
		function(faceIndex, start, 4, data);
	
	start = polygonIndices.data();
	end = start + polygonIndices.size();
//	const uint32_t* size_end = polygonVertexSizes.data() + polygonVertexSizes.size();
	for(const uint32_t* size = polygonVertexSizes.data(); start < end; start += *size, ++size, ++faceIndex)
		function(faceIndex, start, *size, data);
}

std::vector<vec3f> Model::computeFacePoint() const
{
	size_t faceSize = getFaceSize();
	std::vector<vec3f> facePoints;
	facePoints.reserve(faceSize);
	
	for(size_t i = 0, size = triangleIndices.size(); i < size; i += 3)
	{
		const vec3f& v0 = vertices[triangleIndices[i]];
		const vec3f& v1 = vertices[triangleIndices[i + 1]];
		const vec3f& v2 = vertices[triangleIndices[i + 2]];
		facePoints.push_back((v0 + v1 + v2) / 3);
	}
	
	for(size_t i = 0, size = quadrilateralIndices.size(); i < size; i += 4)
	{
		const vec3f& v0 = vertices[quadrilateralIndices[i]];
		const vec3f& v1 = vertices[quadrilateralIndices[i + 1]];
		const vec3f& v2 = vertices[quadrilateralIndices[i + 2]];
		const vec3f& v3 = vertices[quadrilateralIndices[i + 3]];
		facePoints.push_back((v0 + v1 + v2 + v3) / 4);
	}
	
	uint32_t polygonIndex = 0;
	for(const uint32_t& size: polygonVertexSizes)
	{
		assert(size > 4);
		vec3f point(0, 0, 0);
		for(uint32_t i = 0; i < size; ++i)
			point += vertices[polygonIndices[polygonIndex +i]];
		facePoints.push_back(point / size);
		
		polygonIndex += size;
	}
	
	return facePoints;
}

std::vector<float> Model::computeFaceArea() const
{
	size_t faceSize = getFaceSize();
	std::vector<float> areas;
	areas.reserve(faceSize);
	
	for(size_t i = 0, size = triangleIndices.size(); i < size; i += 3)
	{
		const vec3f& v0 = vertices[triangleIndices[i]];
		const vec3f& v1 = vertices[triangleIndices[i + 1]];
		const vec3f& v2 = vertices[triangleIndices[i + 2]];
		float area = 0.5 * cross(v1 - v0, v2 - v0).length();  // s = 1/2 * dot(AB, AC)
		areas.push_back(area);
	}
	
	for(size_t i = 0, size = quadrilateralIndices.size(); i < size; i += 4)
	{
		const vec3f& v0 = vertices[quadrilateralIndices[i]];
		const vec3f& v1 = vertices[quadrilateralIndices[i + 1]];
		const vec3f& v2 = vertices[quadrilateralIndices[i + 2]];
		const vec3f& v3 = vertices[quadrilateralIndices[i + 3]];
		
		vec3 v01 = v1 - v0, v02 = v2 - v0, v03 = v3 - v0;
		float area = 0.5 * (cross(v01, v02).length() + cross(v01, v03).length());
		areas.push_back(area);
	}
	
	uint32_t polygonIndex = 0;
	for(const uint32_t& size: polygonVertexSizes)
	{
		float area = 0;
		const vec3f& v0 = vertices[polygonIndices[polygonIndex]];
		for(uint32_t i = polygonIndex + 2, end = polygonIndex + size; i < end; ++i)
		{
			const vec3f& v1 = vertices[polygonIndices[i - 1]];
			const vec3f& v2 = vertices[polygonIndices[i]];
			
			vec3 v01 = v1 - v0, v02 = v2 - v0;
			area += cross(v01, v02).length();
		}
		
		area *= 0.5;
		areas.push_back(area);
		polygonIndex += size;
	}
	
	assert(areas.size() == faceSize);
	return areas;
}

std::vector<vec3f> Model::computeFaceNormal() const
{
	size_t faceSize = getFaceSize();
	std::vector<vec3f> faceNormals;
	faceNormals.reserve(faceSize);
	
	for(size_t i = 0, size = triangleIndices.size(); i < size; i += 3)
	{
		const vec3f& v0 = vertices[triangleIndices[i]];
		const vec3f& v1 = vertices[triangleIndices[i + 1]];
		const vec3f& v2 = vertices[triangleIndices[i + 2]];
		vec3f product = cross(v1 - v0, v2 - v0);
		// if line v0v1 is parallel to line v0v2, the product is zero, and it cannot be normalized.
		vec3f normal = product.normalize();
		faceNormals.push_back(normal);
	}
	
/*
	0--3
	|\ |
	| \|
	1--2
*/
	for(size_t i = 0, size = quadrilateralIndices.size(); i < size; i += 4)
	{
		const vec3f& v0 = vertices[quadrilateralIndices[i]];
		const vec3f& v1 = vertices[quadrilateralIndices[i + 1]];
		const vec3f& v2 = vertices[quadrilateralIndices[i + 2]];
		const vec3f& v3 = vertices[quadrilateralIndices[i + 3]];
		
		vec3 v01 = v1 - v0, v02 = v2 - v0, v03 = v3 - v0;
		vec3f normal102 = cross(v01, v02);
		vec3f normal203 = cross(v02, v03);
		// TODO: Note the order 0123, namely, 1, 3 should be on each side of line 02.
		// the larger area, the bigger weight.
		vec3f normal = (normal102 + normal203).normalize();
		faceNormals.push_back(normal);
	}
	
	uint32_t polygonIndex = 0;
	for(const uint32_t& size: polygonVertexSizes)
	{
#if 1
		vec3f normal(0, 0, 0);
		const vec3f& v0 = vertices[polygonIndices[polygonIndex]];
		for(uint32_t i = polygonIndex + 2, end = polygonIndex + size; i < end; ++i)
		{
			const vec3f& v1 = vertices[polygonIndices[i -1]];
			const vec3f& v2 = vertices[polygonIndices[i]];
			
			vec3 v01 = v1 - v0, v02 = v2 - v0;
			normal += cross(v01, v02);
		}
		
		normal.normalize();
#else
		// least mean square method
		// many points fit plane a*x + b*y + c*z + d = 0;
		// AX = B  =>  A^T A X = A^T B  => X = (A^T A)^-1 A^T B
		// https://www.ilikebigbits.com/2015_03_04_plane_from_points.html
		
#endif
		faceNormals.push_back(normal);
		polygonIndex += size;
	}
	
	assert(faceNormals.size() == faceSize);
	return faceNormals;
}

std::vector<std::string> Model::getGroupNames() const
{
	std::vector<std::string> names;
	names.reserve(groups.size());
	for(const auto &[name, group]: groups)
		names.push_back(name);
	
	return names;
}

void Model::removeGroup(const std::string& name)
{
	groups.erase(name);
}

template <typename T>
std::unordered_map<T, uint32_t> unique(const std::vector<T>& elements)
{
	std::unordered_map<T, uint32_t> elementTable;
	uint32_t index = 0;
	for(const T& element: elements)
	{
		auto it = elementTable.find(element);
		if(it == elementTable.end())
			elementTable.emplace(element, index++);
	}
	
	return elementTable;
}

void Model::addGroup(const std::string& name, const Group& group)
{
	const auto& [it, successful] = groups.emplace(name, group);
	if(successful)
	{
		std::vector<uint32_t>& indices = it->second.indices;
		std::sort(indices.begin(), indices.end());
	}
}

void Model::addGroup(const std::string& name, Group&& group)
{
	std::vector<uint32_t>& indices = group.indices;
	std::sort(indices.begin(), indices.end());
	groups.emplace(name, std::move(group));
}

bool Model::findGroup(const std::string& name, Group& group) const
{
	auto it = groups.find(name);
	if(it != groups.end())
	{
		group = it->second;
		return true;
	}
	
	return false;
}

/*
void Model::removeDoubles()
{
	std::unordered_map<vec3f, uint32_t> vertexTable   = unique(vertices);
	std::unordered_map<vec2f, uint32_t> texcoordTable = unique(texcoords);
	std::unordered_map<vec3f, uint32_t> normalTable   = unique(normals);
	
	
	std::vector<uint32_t> indexMap;
	map.reserve(vertexTable.size());
	
	// TODO: vertex in mesh
	
	if(vertexTable.size() < vertices.size())
	{
		for(const vec3f& vertex: vertices)
			indexMap.push_back(vertexTable[vertex]);
		
		for(uint32_t& index: triangleIndices)
			index = indexMap[index];
		for(uint32_t& index: quadrilateralIndices)
			index = indexMap[index];
		for(uint32_t& index: polygonIndices)
			index = indexMap[index];
	}
	
	map.clear();
	
	if(vertices.size() > vertexTable.size())
		slog.i(TAG, "remove duplicate vertex %zu => %zu", vertices.size(), vertexTable.size());
	if(texcoords.size() > texcoordTable.size())
		slog.i(TAG, "remove duplicate texcoord %zu => %zu", texcoords.size(), texcoordTable.size());
	if(normals.size() > normalTable.size())
		slog.i(TAG, "remove duplicate normal %zu => %zu", normals.size(), normalTable.size());
}
*/

uint32_t Model::findEdge(uint32_t vertex0, uint32_t vertex1, uint32_t faces[2]) const
{
	assert(vertex0 != vertex1);
	class Closure
	{
	public:
		const uint32_t& vertex0;
		const uint32_t& vertex1;
		
		uint32_t* const faces;
		uint32_t faceFound = 0;
	
	public:
		Closure(const uint32_t& vertex0, const uint32_t& vertex1, uint32_t* faces):
				vertex0(vertex0), vertex1(vertex1), faces(faces)
		{
		}
		
		void search(uint32_t faceIndex, const uint32_t* array, uint32_t length)
		{
			for(uint32_t i = 0; i < length; ++i)
			{
				if(array[i] != vertex0 || 
						(array[(i + 1) % length] != vertex1 && array[(i + length - 1) % length] != vertex1))
					continue;
				
				if(faceFound < 2)
				{
					faces[faceFound] = faceIndex;
					++faceFound;
				}
				
				break;
			}
		}
	};
	
	auto search = [](uint32_t faceIndex, const uint32_t* array, uint32_t length, void* data)
	{
		static_cast<Closure*>(data)->search(faceIndex, array, length);
	};

	Closure closure(vertex0, vertex1, faces);
	traverseFace(search, &closure);
	return closure.faceFound;
}

Model Model::subdivide() const
{
	// 1. Add a new point to each face, called the face-point.
	std::vector<vec3f> facePoints = computeFacePoint();
	const size_t faceSize = facePoints.size();
	
	// 2. Add a new point to each edge, called the edge-point.
	std::vector<uint32_t> edges = getEdgeIndex();
	const size_t edgeSize = edges.size() >> 1;
	std::vector<vec3f> edgePoints(edgeSize);
	std::unordered_map<uint64_t, uint32_t> edgeMap;
	edgeMap.reserve(edgeSize);
	for(size_t i = 0; i < edgeSize; ++i)
	{
		uint32_t vertex0 = edges[i << 1];
		uint32_t vertex1 = edges[(i << 1) + 1];
		uint32_t faces[2];
		uint32_t faceFound = findEdge(vertex0, vertex1, faces);
		if(faceFound == 2)
			edgePoints[i] = (vertices[vertex0] + vertices[vertex1] + facePoints[faces[0]] + facePoints[faces[1]]) / 4;
		else
			slog.w(TAG, "not manifold face, line with vertex #%" PRId32 " and #%" PRId32 " has %" PRId32 " face",
					vertex0, vertex1, faceFound);
		
		edgeMap.emplace(makeEdge(vertex0, vertex1), i);
	}
	
	const size_t vertexSize = vertices.size(); 
	slog.d(TAG, "size: vertex %zu, edge %zu, face %zu\n", vertexSize, edgeSize, faceSize);

	// 3. Move the control-point to another position, called the vertex-point.
	// Q/n + 2R/n + (n-3)S/n
	// The valence of a point is simply the number of edges that connect to that point.
	// where n is the valence, Q is the average of the surrounding face points, R is the average of 
	// all surround edge midpoints, and S is the original control point.
	std::vector<vec3f> vertices1(vertexSize);
	const size_t triangleSize = triangleIndices.size();
	const size_t quadrilateralSize = quadrilateralIndices.size();
	const size_t polygonSize = polygonIndices.size();
	for(size_t i = 0; i < vertexSize; ++i)
	{
		vec3f adjacentFacePoint(0, 0, 0), adjacentEdgePoint(0, 0, 0);
		uint32_t adjacentFace = 0;
		uint32_t faceStart = 0;
		for(size_t j = 0; j < triangleSize; ++j)
		{
			if(triangleIndices[j] != i)
				continue;
			
			adjacentFacePoint += facePoints[j / 3];
			
			size_t start = j / 3 * 3;
			const vec3f& v0 = vertices[triangleIndices[j]];
			const vec3f& v1 = vertices[triangleIndices[start + (j - start + 1) % 3]];
			const vec3f& v2 = vertices[triangleIndices[start + (j - start + 2) % 3]];
			adjacentEdgePoint += v0 + (v1 + v2) * 0.5F;
			
			++adjacentFace;
		}
		
		faceStart += triangleIndices.size() / 3;
		for(size_t j = 0; j < quadrilateralSize; ++j)
		{
			if(quadrilateralIndices[j] != i)
				continue;
			
			adjacentFacePoint += facePoints[faceStart + j / 4];
			
			size_t start = j & ~3;
			const vec3f& v0 = vertices[quadrilateralIndices[j]];
			const vec3f& v1 = vertices[quadrilateralIndices[start + ((j - start + 1) & 3)]];
			const vec3f& v2 = vertices[quadrilateralIndices[start + ((j - start + 3) & 3)]];
			adjacentEdgePoint += v0 + (v1 + v2) * 0.5F;
			
			++adjacentFace;
		}
		
		faceStart += quadrilateralIndices.size() / 4;
		for(size_t j = 0, k = 0; j < polygonSize; j += polygonVertexSizes[k], ++k)
		{
			const uint32_t N = polygonVertexSizes[k];
			for(uint32_t n = 0; n < N; ++n)
			{
				if(polygonIndices[j + n] != i)
					continue;
				
				adjacentFacePoint += facePoints[faceStart + k];
				
				const vec3f& v0 = vertices[polygonIndices[j + n]];
				const vec3f& v1 = vertices[polygonIndices[j + (n + 1) % N]];
				const vec3f& v2 = vertices[polygonIndices[j + (n + N - 1) % N]];
				adjacentEdgePoint += v0 + (v1 + v2) * 0.5F;
				
				++adjacentFace;
				break;
			}
		}
		
		vertices1[i] = (adjacentFacePoint + adjacentEdgePoint + (adjacentFace - 3) * vertices[i]) / adjacentFace;
	}
	
	// 4. Connect the new points.
	for(const vec3f& vertex: edgePoints)
		vertices1.push_back(vertex);
	for(const vec3f& vertex: facePoints)
		vertices1.push_back(vertex);
	
	std::vector<uint32_t> triangles1;
	std::vector<uint32_t> quadrilaterals1;
/*
	     v0                  v0                  v0
	     /\                  /\                  /\
	    /  \                /  \              n2/  \n1
	   /    \            n2/____\n1            /\  /\
	  /      \            /\    /\            /  n3  \
	 /        \          /  \  /  \          /   |    \
	/__________\        /____\/____\        /____|_____\
	v1         v2       v1   n0    v2       v1   n0    v2
*/
	auto pushTriangle = [](std::vector<uint32_t> indices, uint32_t v0, uint32_t v1, uint32_t v2)
	{
		indices.push_back(v0);
		indices.push_back(v1);
		indices.push_back(v2);
	};
	
	auto pushQuadrilateral = [](std::vector<uint32_t> indices, uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3)
	{
		indices.push_back(v0);
		indices.push_back(v1);
		indices.push_back(v2);
		indices.push_back(v3);
	};
	
	uint32_t startIndex = vertexSize + edgePoints.size();
	for(size_t i = 0; i < triangleSize; i += 3)
	{
		uint32_t v0 = triangleIndices[i], v1 = triangleIndices[i + 1], v2 = triangleIndices[i + 2];
		uint32_t n0 = vertexSize + edgeMap[makeEdge(v1, v2)];
		uint32_t n1 = vertexSize + edgeMap[makeEdge(v2, v0)];
		uint32_t n2 = vertexSize + edgeMap[makeEdge(v0, v1)];
#if 0
		pushTriangle(triangles1, v0, n2, n1);
		pushTriangle(triangles1, n2, v1, n0);
		pushTriangle(triangles1, n1, n0, v2);
		pushTriangle(triangles1, n0, n1, n2);
#else
		uint32_t n3 = startIndex + i / 3;
		pushQuadrilateral(quadrilaterals1, v0, n2, n3, n1);
		pushQuadrilateral(quadrilaterals1, v1, n0, n3, n2);
		pushQuadrilateral(quadrilaterals1, v2, n1, n3, n0);
#endif
	}
	
/*
	v1___________v0      v1_____n1____v0
	 |           |        |     |     |
	 |           |        |     |     |
	 |           |      n2|_____n4____|n0
	 |           |        |     |     |
	 |           |        |     |     |
	 |___________|        |_____|_____|
	v2           v3      v2     n3    v3
*/
	startIndex += triangleIndices.size() / 3;
	for(size_t i = 0; i < quadrilateralSize; i += 4)
	{
		uint32_t v0 = triangleIndices[i], v1 = triangleIndices[i + 1];
		uint32_t v2 = triangleIndices[i + 2], v3 = triangleIndices[i + 3];
		uint32_t n0 = vertexSize + edgeMap[makeEdge(v0, v3)];
		uint32_t n1 = vertexSize + edgeMap[makeEdge(v1, v0)];
		uint32_t n2 = vertexSize + edgeMap[makeEdge(v2, v1)];
		uint32_t n3 = vertexSize + edgeMap[makeEdge(v3, v2)];
		uint32_t n4 = startIndex + (i >> 2);
		pushQuadrilateral(quadrilaterals1, v0, n1, n4, n0);
		pushQuadrilateral(quadrilaterals1, v1, n2, n4, n1);
		pushQuadrilateral(quadrilaterals1, v2, n3, n4, n2);
		pushQuadrilateral(quadrilaterals1, v3, n0, n4, n3);
	}
	
	// TODO: support ngon
	
	Model model1;
	model1.vertices = std::move(vertices1);
//	model1.texcoords = 
//	model1.normals =
	model1.triangleIndices = triangles1;
	model1.quadrilateralIndices = quadrilaterals1;
	model1.polygonIndices = polygonIndices;
	model1.polygonVertexSizes = polygonVertexSizes;
	return model1;
}

void Model::triangulate(TriangulationMethod method)
{
	auto pushTriangle = [&triangleIndices = triangleIndices](uint32_t i, uint32_t j, uint32_t k)
	{
		triangleIndices.push_back(i);
		triangleIndices.push_back(j);
		triangleIndices.push_back(k);
	};
	
	for(size_t i = 0, size = quadrilateralIndices.size(); i < size; i += 4)
	{
		//  3---2    3---2
		//  |  /|    |\  |
		//  | / |    | \ |
		//  |/  |    |  \|
		//  0---1    0---1
		auto cut02 = [&pushTriangle, &i]() { pushTriangle(i, i + 1, i + 2);  pushTriangle(i,     i + 2, i + 3); };
		auto cut13 = [&pushTriangle, &i]() { pushTriangle(i, i + 1, i + 3);  pushTriangle(i + 1, i + 2, i + 3); };
		switch(method)
		{
		case TriangulationMethod::FIXED:
			cut02();
			break;
		case TriangulationMethod::ALTERNATE:
			cut13();
			break;
		case TriangulationMethod::BEAUTY:
			{
				const vec3f& v0 = vertices[quadrilateralIndices[i]];
				const vec3f& v1 = vertices[quadrilateralIndices[i + 1]];
				const vec3f& v2 = vertices[quadrilateralIndices[i + 2]];
				const vec3f& v3 = vertices[quadrilateralIndices[i + 3]];
				/*
				assert(cross(v1 - v0, v2 - v1) >= 0);
				assert(cross(v2 - v1, v3 - v2) >= 0);
				assert(cross(v3 - v2, v0 - v3) >= 0);
				*/
				vec3f v10 = (v0 - v1).normalize(), v12 = (v2 - v1).normalize();
				vec3f v30 = (v0 - v3).normalize(), v32 = (v2 - v3).normalize();
				float angle012 = std::acos(dot(v10, v12));
				float angle032 = std::acos(dot(v30, v32));
				if(angle012 + angle032 <= M_PI)  // alpha + beta <= pi meets the Delaunay condition. 
					cut02();
				else
					cut13();
			}
			break;
		case TriangulationMethod::SHORT_EDGE:
			{
				const vec3f& v0 = vertices[quadrilateralIndices[i]];
				const vec3f& v1 = vertices[quadrilateralIndices[i + 1]];
				const vec3f& v2 = vertices[quadrilateralIndices[i + 2]];
				const vec3f& v3 = vertices[quadrilateralIndices[i + 3]];
				
				float edge02 = (v0 - v2).length2();
				float edge13 = (v1 - v3).length2();
				if(edge02 <= edge13)
					cut02();
				else
					cut13();
			}
			break;
		default:
			assert(false);
			break;
		}
	}
	quadrilateralIndices.clear();
	quadrilateralIndices.shrink_to_fit();
	
	uint32_t index = 0;
	for(const uint32_t& number: polygonVertexSizes)
	{
		assert(number > 4);
		switch(method)
		{
		case TriangulationMethod::FIXED:
			for(uint8_t k = 2; k < number; ++k)  // n - 2 triangles
				pushTriangle(polygonIndices[index], polygonIndices[index + k - 1], polygonIndices[index + k]);
			break;
		case TriangulationMethod::BEAUTY:
		default:
			// TODO:
			break;
		}
		
		index += number;
	}
	
	polygonIndices.clear();
	polygonIndices.shrink_to_fit();
	polygonVertexSizes.clear();
	polygonVertexSizes.shrink_to_fit();
}

bool Model::areFacesTriangulated() const
{
	return quadrilateralIndices.empty() && polygonIndices.empty();
}
#if 0
bool Model::calculateNormal(bool perVertex, bool force/* = false*/)
{
	bool hasNormal = !normals.empty();
	if(!force && hasNormal)
		return false;
	
	if(force)
	{
		for(Group& group: groups)
			group.normalIndices.clear();
		normals.clear();
	}
	
	constexpr float LENGTH_LOWER_LIMIT = std::numeric_limits<float>::epsilon();
	if(perVertex)
	{
		const size_t vertexCount = vertices.size();
		normals.resize(vertexCount);
		std::vector<uint32_t> vertexShared(vertexCount);
		
		for(Group& group: groups)
		{
			std::vector<uint32_t>& vertexIndices = group.vertexIndices;
			size_t i = 0;
			for(const uint8_t& n: group.polygons)
			{
				const vec3f& vertex0 = vertices[vertexIndices[i]];
				const vec3f& vertex1 = vertices[vertexIndices[i + 1]];
				vec3f vector1 = vertex1 - vertex0;
				
				vec3f product;
				float length;
				uint8_t v = 2;
				for(; v < n; ++v)
				{
					const vec3f& vertex2 = vertices[vertexIndices[i + v]];
					vec3f vector2 = vertex2 - vertex0;
					product = cross(vector1, vector2);
					
					length = product.length();
					if(length > LENGTH_LOWER_LIMIT)
						break;
				}
				
//				if(v >= n)
//					slog.w(TAG, "perVertex=%d, found bad face. i=%d", perVertex, i);
				
				vec3f normal = product / length;  // product.normalize();
				assert(!std::isnan(normal.x) && !std::isnan(normal.y) && !std::isnan(normal.z));
				for(size_t v = 0; v < n; ++v)
				{
					size_t vertex_index = vertexIndices[i + v];
					normals[vertex_index] += normal;
					++vertexShared[vertex_index];
				}
				i += n;
			}
		}
		
		for(size_t i = 0; i < vertexCount; ++i)
		{
			assert(vertexShared[i] > 0);
			normals[i] /= vertexShared[i];
		}
		// now normal and vertex have common indexing.
		for(Group& group: groups)
			group.normalIndices = group.vertexIndices;
	}
	else
	{
		const size_t faceSize = getFaceSize();
		normals.resize(faceSize);
		
		uint32_t faceIndex = 0;
		for(Group& group: groups)
		{
			const std::vector<uint32_t>& vertexIndices = group.vertexIndices;
			std::vector<uint32_t>& normalIndices = group.normalIndices;
			size_t i = 0;
			for(const uint8_t& n: group.polygons)
			{
				const vec3f& vertex0 = vertices[vertexIndices[i]];
				const vec3f& vertex1 = vertices[vertexIndices[i + 1]];
				vec3f vector1 = vertex1 - vertex0;
				
				vec3f product;
				float length;
				uint8_t v = 2;
				for(; v < n; ++v)
				{
					const vec3f& vertex2 = vertices[vertexIndices[i + v]];
					vec3f vector2 = vertex2 - vertex0;
					product = cross(vector1, vector2);
					length = product.length();
					if(length > LENGTH_LOWER_LIMIT)
						break;
				}
				
//				if(v >= n)
//					slog.w(TAG, "perVertex=%d, found bad face #%" PRId32, perVertex, faceIndex);
				
				vec3f normal = product / length;  // product.normalize();
				assert(!std::isnan(normal.x) && !std::isnan(normal.y) && !std::isnan(normal.z));
				normals[faceIndex] = normal;
				for(uint8_t i = 0; i < n; ++i)
					normalIndices.push_back(faceIndex);
				++faceIndex;
				
				i += n;
			}
		}
	}
	
	return true;
}
#endif
#if 0
std::unique_ptr<Mesh> Model::extract() const
{
	auto isSameGroupName = [&groupName](const Group& group) {return group.name == groupName;};
	auto it = std::find_if(groups.begin(), groups.end(), isSameGroupName);
	if(it == groups.end())
		return {};
	
	size_t faceCount0 = it->polygons.size();
	const Group& group = it->isFaceTriangulated()? *it: triangulateFace(*it, TriangulationMethod::BEAUTY);
	size_t faceCount1 = group.polygons.size();
	slog.d(TAG, "face count from %zu to %zu", faceCount0, faceCount1);

	const bool hasTexcoord = !texcoords.empty();
	const bool hasNormal = !normals.empty();
	const size_t vertexCount = group.vertexIndices.size();
	
	std::vector<vec3f> meshVertices;
	std::vector<vec2f> meshTexcoords;
	std::vector<vec3f> meshNormals;
	
	meshVertices.reserve(vertexCount);
	meshTexcoords.reserve(vertexCount);
	meshNormals.reserve(vertexCount);
	
	for(size_t i = 0; i < vertexCount; ++i)
	{
		meshVertices.push_back(vertices[group.vertexIndices[i]]);
		if(hasTexcoord)
			meshTexcoords.push_back(texcoords[group.texcoordIndices[i]]);
		if(hasNormal)
			meshNormals.push_back(normals[group.normalIndices[i]]);
	}
	
	std::unordered_map<std::string, std::shared_ptr<Texture>> textureMap;
	if(materials != nullptr)
		textureMap = materials->loadTexture();
	
	std::shared_ptr<Mesh> mesh = Mesh::Builder(std::move(meshVertices))
			.setTexcoord(std::move(meshTexcoords))
			.setNormal(std::move(meshNormals))
//			.setTexture(std::move(meshTextures))
			.build();
	mesh->setName(group.name);
	
	std::vector<std::shared_ptr<Texture>> textures;
	for(std::pair<const std::string, std::shared_ptr<Texture>>& pair: textureMap)
		textures.emplace_back(pair.second);
	mesh->setTexture(textures);
	slog.d(TAG, "mesh %s size: vertex=%zu, texcoord=%zu, normal=%zu", name.c_str(), vertices.size(), texcoords.size(), normals.size());
	if(materials != nullptr && materials->contains(group.materialName))
	{
		std::shared_ptr<Material> material = materials->at(group.materialName);
		mesh->setMaterial(material.get());
	}
	
	return mesh;
}
#endif
