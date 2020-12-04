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
private:
	const std::string& path;
	std::string name;

	std::vector<vec3f> vertices;
	std::vector<vec2f> texcoords;
	std::vector<vec3f> normals;
	
	std::vector<vec3u> indices;  ///< v/vt/vn triplet, base 1 indexed, 0 is is a placeholder.
	std::vector<uint32_t> faceVertexSizes;
	
	std::vector<Group> groupArray;
	std::map<std::string, std::set<uint32_t>> groups;

//	std::unordered_map<std::string, Mesh*> meshes;
	
	Model_MTL* materials;
//	std::vector<Group> groups;
//	std::vector<vec3f> parameterized

private:
	/**
	 * @param[in] path
	 * @param[in] materialFileName mtllib name, leave it empty if it doesn't have a name.
	 * @param[in] baseIndex .OBJ file starts with index 1.
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
/*
	Model_OBJ(Model_OBJ&& other) = default;
	Model_OBJ& operator =(Model_OBJ&& other) = default;
*/
	/**
	 * set object's name
	 * @param[in] name the object's name.
	 */
	void setName(const std::string& name);
	std::string getName() const;
	
	const std::string& getPath() const;
	
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
	
	void clear();
};

inline void        Model_OBJ::setName(const std::string& name) { this->name = name; }
inline std::string Model_OBJ::getName() const                  { return name;       }

}  // namespace pea
#endif  // PEA_IO_MODEL_OBJ_H_
