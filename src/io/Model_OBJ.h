#ifndef PEA_IO_MODEL_OBJ_H_
#define PEA_IO_MODEL_OBJ_H_

#include <set>
#include <string>
#include <map>
#include <memory>
#include <vector>

#include "math/vec2.h"
#include "math/vec3.h"
#include "io/Group.h"
#include "io/Model.h"
#include "io/Model_MTL.h"

namespace pea {

/**
 * OBJ is a widely used de facto standard in the 3D industry. The OBJ format is a popular plain text
 * format, however, it has only basic geometry and material support. There is no support for mesh 
 * vertex colors, armatures, animation, lights, cameras, empty objects, parenting, or 
 * transformations.
 *
 * Wavefront .obj file specification @see {@link http://www.martinreddy.net/gfx/3d/OBJ.spec }
 * https://wiki.fileformat.com/3d/obj/
 * https://docs.blender.org/manual/en/2.81/addons/import_export/io_scene_obj.html
 */
class Model_OBJ
{
public:

private:
	enum Key
	{
		KEY_MATERIALS = 0,  // "mtllib"
		KEY_USE_MATERIAL,   // "usemtl"
		KEY_OBJECT_NAME,    // "o"
		KEY_VERTEX,         // "v"
		KEY_TEXTURE,        // "vt"
		KEY_NORMAL,         // "vn"
		KEY_FACE,           // "f"
//		static const char KEY_GROUP_NAME = "g";
//		static const char KEY_OBJECT_NAME = "o";
//		static const char KEY_SMOOTHING_GROUP = "s";
//		static const char KEY_POINT = "p";
//		static const char KEY_LINE = "l";
//		static const char KEY_MAPLIB = "maplib";
//		static const char KEY_USEMAP = "maplib";
		KEY_COUNT
	};

	const std::string& path;
	std::string name;

	std::vector<vec3f> vertices;
	std::vector<vec2f> texcoords;
	std::vector<vec3f> normals;
	
	std::vector<vec3u> indices;
	std::vector<uint32_t> faceVertexSizes;
	

	std::vector<Group> groupArray;
	std::map<std::string, std::set<uint32_t>> groups;

//	std::unordered_map<std::string, Mesh*> meshes;
	
	Model_MTL* materials;
//	std::vector<Group> groups;
//	std::vector<vec3f> parameterized
//	static const Property property[KEY_COUNT];
	
private:
	static bool check(const char*& str, Key key);
/*
	static uint32_t updateVertexInfo(Mesh* mesh,
		std::unordered_map<vec3u, size_t>& triple_cache,
		const vec3u& index,
		const std::vector<vec3f>& in_vertices,
		const std::vector<vec2f>& in_texcoords,
		const std::vector<vec3f>& in_normals);

	static bool triangulateFace(Mesh* mesh,
			std::unordered_map<vec3u, size_t>& vertex_cache,
			const std::vector<vec3f>& in_vertices,
			const std::vector<vec2f>& in_texcoords,
			const std::vector<vec3f>& in_normals,
			const std::vector<std::vector<vec3u>>& face_group);
*/
	bool save_OBJ(const std::string& path, const std::string& materialFileName, const vec3u& baseIndex) const;

	void addGroup(const std::string& name, const Group& group);
public:
	/**
	 * this will assume one object per file, use load() if you are not sure.
	 */
	explicit Model_OBJ(const std::string& path) noexcept(false);
	explicit Model_OBJ(const Model& model) noexcept;
	~Model_OBJ();
	
	Model_OBJ(const Model_OBJ& other) = delete;
	Model_OBJ& operator =(const Model_OBJ& other) = delete;
	
//	Model_OBJ(Model_OBJ&& other) = default;
//	Model_OBJ& operator =(Model_OBJ&& other) = default;
	
//	void shrink();
	
	/**
	 * set object's name
	 * @param[in] name the object's name.
	 */
	void setName(const std::string& name);
	std::string getName() const;
	
	const std::string& getPath() const;
	
//	static std::shared_ptr<Model_OBJ> load(const std::string& path);
	
	/**
	 * Model class is used for editing, index data will be dropped.
	 * Mesh class is used for rendering.
	 */
	std::shared_ptr<Model> exportModel() const;

	/**
	 * Save .OBJ and .MTL file
	 */
	bool save(const std::string& dir, const std::string& name) const;
	
	/**
	 * Save .OBJ file
	 * @param[in] path Relative or absolute path of the object.
	 * @param[in] materialFileName mtllib name, or material path's basename. you can specify 
	 *            multiple names, separated by space.
	 */
	bool save_OBJ(const std::string& path, const std::string& materialFileName) const;
	
	/**
	 * Save .MTL file
	 */
	bool save_MTL(const std::string& path) const;


	/**
	 * "usrmtl name" in .OBJ specification, search name in .MTL file list in
	 * "mtllib filename1 filename2 ..." statement
	 */
//	const Material& find(const std::string& name) const;


	bool parse(const std::string& path);

	void clear();
};

inline void        Model_OBJ::setName(const std::string& name) { this->name = name; }
inline std::string Model_OBJ::getName() const                  { return name;       }

}  // namespace pea
#endif  // PEA_IO_MODEL_OBJ_H_
