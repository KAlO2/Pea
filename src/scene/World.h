#ifndef PEA_SCENE_WORLD_H_
#define PEA_SCENE_WORLD_H_

#include <vector>

#include "scene/Camera.h"
#include "view/Window.h"

namespace pea {

// forward declaritions
class Mesh;
class Skybox;
class Texture;
class Floor;


class World: public Window
{
private:
	Camera camera;
	std::vector<Mesh*> objects;
	Skybox* skybox;
	Floor* floor;
	
	float velocity;
	enum class HumanState
	{
		ON_THE_GROUND,
		IN_THE_AIR,
		CROUCH,
	};
	HumanState humanState;
	
protected:
	float deltaTime;

protected:
	void setState(HumanState state);
	void stand(float dt);
	
	void onKey(int32_t key, int32_t scanCode, int32_t action, int32_t mods) override;
	
	void onWindowResize(int32_t width, int32_t height) override;
	
	void onWindowFocusChanged(bool focused) override;
	
	void onMouseMove(float dx, float dy) override;
	
	void onMouseScroll(float dx, float dy) override;
	
	void render() override;
	
	void update(float dt) override;
	
	/**
	 * if program use viewPosition, you call this method to update view position before rendering.
	 */
	void updateViewPositionUniform();
	
	void render(const mat4f& view, const mat4f& projection) const;
	
public:
	World(int32_t width, int32_t height, const char* title);
	virtual ~World();
	
	World(const World& other) = delete;
	World& operator = (const World& other) = delete;

	Camera& getCamera();
	
	void loadSkyBox(const std::string& dir, const std::string filenames[6]);
	
	virtual void prepare();
	
	const Texture* getSkyboxTexture() const;
	
	size_t getObjectArraySize() const;
	Mesh* getObject(size_t index);
	
	void addObject(Mesh* object);
	Mesh* findObject(const std::string& name);
	Mesh* findObject(uint32_t id);
	bool removeObject(const Mesh* object);
};

inline size_t  World::getObjectArraySize() const { return objects.size(); }
inline Mesh*   World::getObject(size_t index)    { return objects[index]; }
inline Camera& World::getCamera()                { return camera;         }

}  // namespace pea
#endif  // PEA_SCENE_WORLD_H_
