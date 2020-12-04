#include "io/Model_OBJ.h"

#include <algorithm>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <unordered_map>

#include "io/FileSystem.h"
#include "io/TypeUtility.h"
#include "io/Model_OBJ.private.h"
#include "io/Model.h"
#include "math/hash.h"
#include "scene/Mesh.h"
#include "scene/Material.h"
#include "util/Log.h"


using namespace pea;

static const char* TAG = "Model_OBJ";
static const std::string EMPTY_PATH;

Model_OBJ::Model_OBJ(const std::string& path) noexcept(false):
		path(path),
		materials(nullptr)
{
	std::ifstream stream(path, std::ios::binary);
	if(!stream.is_open())
		throw std::invalid_argument("could not open path for read. path=" + path);
	
	uint32_t faceIndex = 0;
	Group group(GroupType::FACE);
	
	constexpr size_t N = 4096;
	char buffer[N];
	const char* p = nullptr;
	while(stream.peek() != EOF)
	{
		stream.getline(buffer, N);

		// trim newline '\r\n' or '\n'
		size_t len = std::strlen(buffer);
		if(len > 0 && (buffer[len - 1] == '\n'))
		{
			--len;
			buffer[len] = '\0';
		}
		if(len > 0 && (buffer[len - 1] == '\r'))
		{
			--len;
			buffer[len] = '\0';
		}

		// skip leading space.
		p = buffer + std::strspn(buffer, " \t");

		if(buffer[0] == '\0')  // empty line
			continue;
		if(buffer[0] == '#')  // comment line
			continue;

		if(MATCH_CHAR('v'))  // vertex
			vertices.push_back(TypeUtility::parseFloat3(p));
		else if(MATCH_TWO_CHAR('v', 't'))  // texture coordinate
			texcoords.push_back(TypeUtility::parseFloat2(p));
		else if(MATCH_TWO_CHAR('v', 'n'))  // normal
			normals.push_back(TypeUtility::parseFloat3(p));  // TODO: wikipedia.org says normals might not be unit vectors?
		else if(MATCH_TWO_CHAR('v', 'p'))  // parameter space vertices
		{
		}
		else if(MATCH_CHAR('f'))
		{
			p += std::strspn(p, " \t");

			uint32_t faceVertexSize = 0;
			while(!isNewLine(*p))
			{
				vec3i index = TypeUtility::parseInt3(p);
				// handles negative vertex reference numbers, -1 means last index.
				if(index[0] < 0)
					index[0] += vertices.size();
				if(index[1] < 0)
					index[1] += texcoords.size();
				if(index[2] < 0)
					index[2] += normals.size();
				
				indices.emplace_back(index[0], index[1], index[2]);
				
				++faceVertexSize;
				p += std::strspn(p, " \t\r");
			}
			
			if(faceVertexSize < 3)
				slog.w(TAG, "bad face. face #%" PRId32 " with %" PRId32 " vertex", faceIndex, faceVertexSize);
			if(!groups.empty())
				group.indices.push_back(faceIndex);
			faceVertexSizes.push_back(faceVertexSize);
			++faceIndex;
		}
		else if(MATCH_STRING("usemtl"))
		{
			std::string materialName(p);
/*
			if(materialTable.find(materialName) != materialTable.end())
				material_id = materialTable[materialName];
			else
				material_id = -1;  // Oops, material is missing!
			mesh.material_id = material_id;
*/
//			const Model_MTL* materials = materials.get();
//			const Material* material = materials != nullptr? materials->at(materialName).get(): nullptr;
//			if(material != nullptr)
				group.setMaterial(materialName);
/*			else
			{
				slog.i(TAG, "material %s is missing", p);
				//mesh->materialName.clear();
			}*/
		}
		else if(MATCH_STRING("mtllib"))
		{
			std::string name(p);
			std::string path_mtl = FileSystem::dirname(path) + FileSystem::SEPERATOR + name;
			try
			{
				materials = new Model_MTL(path_mtl);
			}
			catch(const std::bad_alloc& e)
			{
				slog.e(TAG, "line %d allocation failed: %s", __LINE__, e.what());
			}
			catch(...)
			{
				slog.w(TAG, "failed to load Material file \"%s\" ", path_mtl.c_str());
			}
		}
		else if(MATCH_CHAR('g'))  // group
		{
			if(!group.isEmpty())  // start a new group
			{
				groupArray.push_back(std::move(group));
				group.clear();
			}
			
			std::vector<std::string> groupNames = TypeUtility::split(p, " \t");
			for(const std::string& groupName: groupNames)
			{
				auto it = groups.find(groupName);
				uint32_t groupId = groupArray.size();
				if(it != groups.end())
					it->second.insert(groupId);
				else
				{
					std::set<uint32_t> groupIds;
					groupIds.insert(groupId);
					groups.emplace(groupName, std::move(groupIds));
				}
			}
		}
/*
		else if(MATCH_CHAR('o'))  // object name
		{
			// https://github.com/mrdoob/three.js/issues/8383
			// OBJLoader does not distinguish between `o` and `g` tags.
			std::string previousName = object->getName();
			if(!previousName.empty())  // new object encountered
			{
				objects.push_back(std::move(object));
				object = std::make_shared<Model_OBJ>(path);
			}
			else
			{
				std::string name = std::string(p);
				object->setName(name);
			}
		}
		else if(MATCH_STRING("cstype"))
		{
		}
		else if(MATCH_STRING("parm"))
		{
		}
		else if(MATCH_STRING("deg"))
		{
		}
		else if(MATCH_STRING("end"))
		{
		}
*/
		else if(MATCH_CHAR('s'))  // smooth group
		{
			// https://community.khronos.org/t/how-do-i-use-smoothing-groups-from-obj-files/73070/
		}
		else
		{
			// ignore unknown commands
			slog.v(TAG, "unhandled line \"%s\"", p);
		}
	}
/*
	Quote from doc/OBJ.spec:
	group_name is the name for the group. Letters, numbers, and
	combinations of letters and numbers are accepted for group names.
	The default group name is default.
*/
	if(!group.isEmpty())
	{
//		std::string groupName = "default";
		groupArray.push_back(group);
	}
//	if(object.name.empty())
//		object.name = FileSystem::basename(path);
//	objects.push_back(std::move(object));
	// TODO: return multiple objects.
//	return object;
}

Model_OBJ::Model_OBJ(const Model& model) noexcept:
		path(EMPTY_PATH),
		materials(nullptr)
{
	std::vector<uint32_t> vertexMap, texcoordMap, normalMap;
	vertices  = Group::createIndex(model.vertices, vertexMap);
	texcoords = Group::createIndex(model.texcoords, texcoordMap);
	normals   = Group::createIndex(model.normals, normalMap);

	slog.v(TAG, "+%d Model v.size=%zu, vt.size=%zu, vn.size=%zu", __LINE__, model.vertices.size(), model.texcoords.size(), model.normals.size());
	slog.v(TAG, "+%d Model_OBJ v.size=%zu, vt.size=%zu, vn.size=%zu", __LINE__, vertices.size(), texcoords.size(), normals.size());
	
	const bool hasTexcoord = !texcoords.empty();
	const bool hasNormal = !normals.empty();
	auto emplaceBackIndex = [&](const uint32_t& index)
	{
		// .obj use one based index, and 0 is placeholder.
		uint32_t i = vertexMap[index] + 1;
		uint32_t j = hasTexcoord? texcoordMap[index] + 1: 0;
		uint32_t k = hasNormal? normalMap[index] + 1: 0;
		indices.emplace_back(i, j, k);
	};
	
	const std::vector<uint32_t>& triangleIndices = model.triangleIndices;
	const std::vector<uint32_t>& quadrilateralIndices = model.quadrilateralIndices;
	const std::vector<uint32_t>& polygonIndices = model.polygonIndices;
	const size_t triangleIndexSize = triangleIndices.size();
	const size_t quadrilateralIndexSize = quadrilateralIndices.size();
	const size_t polygonIndexSize = polygonIndices.size();
	indices.reserve(triangleIndexSize + quadrilateralIndexSize + polygonIndexSize);
	slog.d(TAG, "index consists of %zu tri, %zu quad and %zu ngon", triangleIndexSize, quadrilateralIndexSize, polygonIndexSize);

	for(size_t i = 0; i < triangleIndexSize; ++i)
		emplaceBackIndex(triangleIndices[i]);
	
	for(size_t i = 0; i < quadrilateralIndexSize; ++i)
		emplaceBackIndex(quadrilateralIndices[i]);
	
	for(size_t i = 0; i < polygonIndexSize; ++i)
		emplaceBackIndex(polygonIndices[i]);
	
	const size_t triangleSize = triangleIndexSize / 3;
	const size_t quadrilateralSize = quadrilateralIndexSize >> 2;
	const size_t polygonSize = faceVertexSizes.size();
	slog.d(TAG, "%zu vertices, faces = %zu tri + %zu quad + %zu ngon", vertices.size(),
			triangleSize, quadrilateralSize, polygonSize);
	faceVertexSizes.reserve(triangleSize + quadrilateralSize + polygonSize);
	for(size_t i = 0; i < triangleSize; ++i)
		faceVertexSizes.emplace_back(3);
	for(size_t i = 0; i < quadrilateralSize; ++i)
		faceVertexSizes.emplace_back(4);
	for(const uint32_t& n: faceVertexSizes)
		faceVertexSizes.emplace_back(n);

	// .OBJ format only archive face group info
	for(const auto&[name, group]: model.groups)
		if(group.getType() == GroupType::FACE)
			addGroup(name, group);
}

/**
 * A and B are ordered input, A0, intersection, B0 are ordered output.
 * intersection = A and B
 * A = A0 + intersection
 * B = B0 + intersection
 */
static void partitionSet(const std::vector<uint32_t>& A, const std::vector<uint32_t>& B,
		std::vector<uint32_t>& A0, std::vector<uint32_t>& intersection, std::vector<uint32_t>& B0)
{
	auto firstA = A.begin(), lastA = A.end();
	auto firstB = B.begin(), lastB = B.end();
	while(firstA != lastA && firstB != lastB)
	{
		if(*firstA < *firstB)
		{
			A0.push_back(*firstA);
			++firstA;
		}
		else if(*firstA > *firstB)
		{
			B0.push_back(*firstB);
			++firstB;
		}
		else  // if(*firstA == *firstB)
		{
			intersection.push_back(*firstA);
			++firstA;
			++firstB;
		}
	}
	
	if(firstA != lastA)
		std::copy(firstA, lastA, std::back_inserter(A0));
	else if(firstB != lastB)
		std::copy(firstB, lastB, std::back_inserter(B0));
}

void Model_OBJ::addGroup(const std::string& name, const Group& group)
{
	assert(!name.empty() && group.getType() == GroupType::FACE);
	slog.v(TAG, "group %s has %zu indices", name.c_str(), group.indices.size());
	bool hasIntersection = false;
	
	std::vector<uint32_t> indices = group.indices;
	std::set<uint32_t> ids;
	const uint32_t size = groupArray.size();
	for(uint32_t id = 0; id < size; ++id)
	{
		Group& existingGroup = groupArray[id];
		std::vector<uint32_t>& existingIndices = existingGroup.indices;
		std::vector<uint32_t> indices0, intersection, existingIndices0;
		partitionSet(indices, existingIndices, indices0, intersection, existingIndices0);
		
		if(intersection.empty())
			continue;
		hasIntersection = true;
		
		if(indices0.empty() && existingIndices0.empty())  // found the same group
		{
			std::set<uint32_t> ids = {id};
			groups.emplace(name, ids);
		}
		else if(existingIndices0.empty())  // an existing group is a subset of the incoming group
		{
			ids.insert(id);
		}
		else
		{
			// the incoming group is a subset of an existing group
			existingGroup.indices = existingIndices0;
			
			const uint32_t newId = groupArray.size();
			for(std::pair<const std::string, std::set<uint32_t>>& pair: groups)
			{
				std::set<uint32_t>& ids = pair.second;
				if(ids.find(id) != ids.end())
					ids.insert(newId);
			}
			
			Group newGroup = group;
			newGroup.indices = intersection;
			groupArray.push_back(newGroup);
			std::set<uint32_t> ids = {newId};
			groups.emplace(name, ids);
			
			// 3 non-empty parts
			if(!indices0.empty())
			{
				newGroup.indices = indices0;
				groupArray.push_back(newGroup);
				std::set<uint32_t> ids = {newId, newId + 1};
				groups.emplace(name, ids);
			}
		}
		
		indices = indices0;  // kick out intersection part from the incoming group
	}
	
	// if incoming group has no intersection with existing group, push back incoming group directly.
	if(!hasIntersection)
	{
		std::set<uint32_t> ids = {static_cast<uint32_t>(groupArray.size())};
		groupArray.push_back(group);
		groups.emplace(name, ids);
	}
	else if(!ids.empty())
	{
		Group newGroup = group;
		newGroup.indices = indices;
		uint32_t newId = static_cast<uint32_t>(groupArray.size());
		ids.insert(newId);
		groupArray.push_back(newGroup);
		groups.emplace(name, ids);
	}
}

const std::string& Model_OBJ::getPath() const
{
	// return absolute path?
	return path;
}

Model_OBJ::~Model_OBJ()
{
/*	for(const std::pair<std::string, Mesh*>& pair: meshes)
		delete pair.second;
	meshes.clear();
*/
	delete materials;
//	materials = nullptr;
/*
	for(const std::pair<std::string, Material*>& pair: materials)
		delete pair.second;
	materials.clear();
*/
}

std::shared_ptr<Model> Model_OBJ::exportModel() const
{
	slog.v(TAG, "+%d Model_OBJ quantity of v=%zu, vt=%zu, vn=%zu", __LINE__, vertices.size(), texcoords.size(), normals.size());
	
	std::vector<vec3f> modelVertices;
	std::vector<vec2f> modelTexcoords;
	std::vector<vec3f> modelNormals;
	
	std::vector<uint32_t> indexArray;
	std::vector<vec3u> uniqueIndices = Group::createIndex(indices, indexArray);
	const size_t count = uniqueIndices.size();
	if(indices.size() != count)
		slog.d(TAG, "remove duplicated indices from %zu to %zu", indices.size(), count);
	
	// keep vertex order in .obj file
//	std::sort(uniqueIndices.begin(), uniqueIndices.end());
	
	// index array to index map
	std::unordered_map<vec3u, uint32_t> indexMap;
	
	for(size_t i = 0; i < count; ++i)
		indexMap[uniqueIndices[i]] = i;
	
	modelVertices.reserve(count);
	modelTexcoords.reserve(count);
	modelNormals.reserve(count);
	
	const bool hasPosition = count > 0 && uniqueIndices[0][0] > 0;
	const bool hasTexcoord = count > 0 && uniqueIndices[0][1] > 0;
	const bool hasNormal   = count > 0 && uniqueIndices[0][2] > 0;
#if 1
	if(hasPosition)
		for(const vec3u& index: uniqueIndices)
			modelVertices.push_back(vertices[index[0] - 1]);
	
	if(hasTexcoord)
		for(const vec3u& index: uniqueIndices)
			modelTexcoords.push_back(texcoords[index[1] - 1]);
	
	if(hasNormal)
		for(const vec3u& index: uniqueIndices)
			modelNormals.push_back(normals[index[2] - 1]);
#else
	for(const vec3u& index: uniqueIndices)
	{
		// vec3u  0/1/2  v/vt/vn
		// Model_OBJ is base 1 indexed, Model is base 0 indexed.
//		assert(index[0] < vertices.size());
//		assert(index[1] < texcoords.size());
//		assert(index[2] < normals.size());
		if(hasPosition)
			modelVertices.push_back(vertices[index[0] - 1]);
		if(hasTexcoord)
			modelTexcoords.push_back(texcoords[index[1] - 1]);
		if(hasNormal)
			modelNormals.push_back(normals[index[2] - 1]);
	}
#endif
	slog.v(TAG, "+%d Model quantity of v=%zu, vt=%zu, vn=%zu", __LINE__, modelVertices.size(), modelTexcoords.size(), modelNormals.size());

	std::shared_ptr<Model> model = std::make_shared<Model>();
	model->vertices  = std::move(modelVertices);
	model->texcoords = std::move(modelTexcoords);
	model->normals   = std::move(modelNormals);
	
	uint32_t size3 = 0, size4 = 0, sizen = 0;
	for(const uint32_t& faceVertexSize: faceVertexSizes)
	{
		assert(faceVertexSize >= 3);
		if(faceVertexSize == 3)
			++size3;
		else if(faceVertexSize == 4)
			++size4;
		else
			++sizen;
	}
	slog.i(TAG, "triangle.count=%" PRIu32 ", quadrilateral.count=%" PRIu32 ", polygon.count=%" PRIu32, size3, size4, sizen);
	std::vector<uint32_t> modelTriangles;
	std::vector<uint32_t> modelQuadrilaterals;
	std::vector<uint32_t> modelPolygons;
	std::vector<uint32_t> modelPolygonVertexSizes;
	modelTriangles.reserve(size3);
	modelQuadrilaterals.reserve(size4);
	modelPolygons.reserve(sizen * 5);  // at least pentagon
	modelPolygonVertexSizes.reserve(sizen);
	
	uint32_t start = 0;
	for(const uint32_t& faceVertexSize: faceVertexSizes)
	{
		if(faceVertexSize == 3)
			for(uint8_t k = 0; k < 3; ++k, ++start)
				modelTriangles.push_back(indexMap[indices[start]]);
		else if(faceVertexSize == 4)
			for(uint8_t k = 0; k < 4; ++k, ++start)
				modelQuadrilaterals.push_back(indexMap[indices[start]]);
		else
		{
			for(uint8_t k = 0; k < faceVertexSize; ++k, ++start)
				modelPolygons.push_back(indexMap[indices[start]]);
			modelPolygonVertexSizes.push_back(faceVertexSize);
		}
	}
/*
	if(!dropIndex)
	{
		for(uint32_t& index: modelTriangles)
			index = indexMap[index];
		for(uint32_t& index: modelQuadrilaterals)
			index = indexMap[index];
		for(uint32_t& index: modelPolygons)
			index = indexMap[index];
	}
*/
	model->triangleIndices      = std::move(modelTriangles);
	model->quadrilateralIndices = std::move(modelQuadrilaterals);
	model->polygonIndices       = std::move(modelPolygons);
	model->polygonVertexSizes   = std::move(modelPolygonVertexSizes);
	
	return model;
}

bool Model_OBJ::save(const std::string& dir, const std::string& name) const
{
	if(name.empty())
		slog.w(TAG, "object name is empty from input file \"%s\"", path.c_str());
	
	std::string path = dir + FileSystem::SEPERATOR + name;
	std::string path_obj = path + ".obj";
	bool hasMaterial = materials != nullptr && materials->getSize() > 0;
	std::string materialFileName = hasMaterial? (name + ".mtl"): "";
	bool flag = save_OBJ(path_obj, materialFileName);
	if(!flag)
		slog.e(TAG, "failed to save %s.", path_obj.c_str());

	if(hasMaterial)
	{
		std::string path_mtl = path + ".mtl";
		bool flag_mtl = save_MTL(path_mtl);
		if(!flag_mtl)
			slog.e(TAG,  "failed to save %s.", path_mtl.c_str());
		flag &= flag_mtl;  // TODO: a &&= b  => a = a && b?
	}
	
	return flag;
}

bool Model_OBJ::save_OBJ(const std::string& path, const std::string& materialFileName) const
{
	// note .obj file index are one based.
	return save_OBJ(path, materialFileName, vec3u(1u, 1u, 1u));
}


bool Model_OBJ::save_OBJ(const std::string& path, const std::string& materialFileName, const vec3u& baseIndex) const
{
	std::ofstream stream(path);
	if(!stream.is_open())
		return false;

	slog.i(TAG, "vertex size=%zu, texcoord size=%zu, normal size=%zu", vertices.size(), texcoords.size(), normals.size());
	// std::numeric_limits<float>::max_digits10
	stream << std::setiosflags(std::ios::fixed) << std::setprecision(6);

	constexpr char _ = ' ';
	stream << comment << '\n';
	if(!materialFileName.empty())
		stream << "mtllib" << _ << materialFileName << '\n';
	stream << '\n';

	// write object name
	if(!name.empty())
		stream << 'o' << _ << name << '\n';
	
	// write vertex positions
	const size_t vertexSize = vertices.size();
	if(vertexSize > 0)
	{
		stream << '#' << _ << vertexSize << _ << "vertex positions" << '\n';
		for(const vec3f& vertex: vertices)
			PUT_VEC3("v", vertex);
		stream << '\n';
	}
	
	// write texture coordinates
	const size_t texcoordSize = texcoords.size();
	if(texcoordSize > 0)
	{
		stream << '#' << _ << texcoordSize << _ << "texture coordinates" << '\n';
		for(const vec2f& texcoord: texcoords)
			PUT_VEC2("vt", texcoord);
		stream << '\n';
	}

	// write vertex normals
	const size_t normalSize = normals.size();
	if(normalSize > 0)
	{
		stream << '#' << _ << normalSize << _ << "vertex normals" << '\n';
		for(const vec3f& normal: normals)
			PUT_VEC3("vn", normal);
		stream << '\n';
	}
	
	const bool hasTexcoord = texcoordSize > 0;
	const bool hasNormal = normalSize > 0;
	auto printIndex = [&stream, &hasTexcoord, &hasNormal](const vec3u& index)
	{
		assert(hasTexcoord == (index.j > 0));
		assert(hasNormal == (index.k > 0));
		
		stream << index.i;
		if(hasTexcoord || hasNormal)
		{
			stream << '/';
			if(hasTexcoord)
				stream << index.j;
			
			if(hasNormal)
			{
				stream << '/';
				stream << index.k;
			}
		}
	};
	
	if(groups.empty())
	{
		uint32_t vertexCount = 0;
		uint32_t polygonIndex = 0;
		for(const vec3u& index: indices)
		{
			if(vertexCount == 0)
				stream << 'f' << _;
			
			printIndex(index);
			stream << ' ';
			++vertexCount;
			if(vertexCount >= faceVertexSizes[polygonIndex])
			{
				stream << '\n';
				vertexCount = 0;
				++polygonIndex;
			}
		}
	}
	else
	{
		const uint32_t polygonSize = faceVertexSizes.size();
		std::vector<uint32_t> polygonStart(polygonSize), polygonStop(polygonSize);
		for(uint32_t i = 0, start = 0; i < polygonSize; ++i)
		{
			polygonStart[i] = start;
			uint32_t stop = start + faceVertexSizes[i];
			polygonStop[i] = stop;
			start = stop;
		}
		
		const size_t groupArraySize = groupArray.size();
		for(size_t i = 0; i < groupArraySize; ++i)
		{
			stream << 'g' << _;
			for(const std::pair<const std::string, std::set<uint32_t>>& pair: groups)
			{
				const std::set<uint32_t>& groupIds = pair.second;
				if(groupIds.find(i) != groupIds.end())
					stream << pair.first << _;
			}
			stream << '\n';
			
			const Group& group = groupArray[i];
			const std::string& materialName = group.getMaterial();
			if(!materialName.empty())
				stream << "usemtl" << _ << materialName << '\n';
			
			uint32_t smooth = group.getSmooth();
			if(smooth > 0)
				stream << 's' << ' ' << smooth << '\n';
			
			assert(group.getType() == GroupType::FACE);
			const std::vector<uint32_t>& faceIndices = group.indices;
			for(const uint32_t& faceIndex: faceIndices)
			{
				stream << 'f' << _;
				for(uint32_t k = polygonStart[faceIndex], stop = polygonStop[faceIndex]; k < stop; ++k)
				{
					printIndex(indices[k]);
					if(k + 1 != stop)
						stream << ' ';  // indices i/j/k are seperated by space
				}
				stream << '\n';
			}
		}
		stream << '\n';
	}
	
	stream.close();
	return true;
}

bool Model_OBJ::save_MTL(const std::string& path) const
{
	if(materials == nullptr)
		return false;
	
	return materials->save(path);
}

/**
 * @brief parse a node in a .obj file, restricted to the following format:
 *   i, i/j, i/j/k, i//k
 * note that those index are counted from index 1.
 * There is no space between numbers and the slashes.
 * @param tuple the trimmed string, formats are list above.
 * @return ternary index value, 0 index for value's not being set.
 */
vec3i parseTriple(const char* &token)
{
	int32_t i = 0, j = 0, k = 0;  // invalid index

	int n = sscanf(token, "%d", &i);  // i
	assert(n > 0);  // at least one number is read
	token += n;
	if(*token == '/')
	{
		++token;
		if(*token == '/')
		{
			++token;
			n = sscanf(token, "%d", &k);  // i//k
			token += n;
		}
		else
		{
			n = sscanf(token, "%d", &j);  // i/j
			token += n;
			if(*token == '/')
			{
				++token;
				n = sscanf(token, "%d", &k);  // i/j/k
				token += n;
			}
		}
	}

	// trailing blank character(s) like space or tab, if exist.
	size_t length = strcspn(token, " \t");
	assert(token[length] == '\0');

	// .OBJ's file index are one based.
	return vec3i(i - 1, j - 1, k - 1);
}
