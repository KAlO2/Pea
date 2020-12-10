#ifndef PEA_SCENE_SKY_BOX_H_
#define PEA_SCENE_SKY_BOX_H_

#include <string>

#include "math/mat4.h"

namespace pea {

class Image;
class Texture;

/**
 * https://en.wikipedia.org/wiki/Skybox_(video_games)
 * SkyBox, rendered with Z buffer turned off, before all other nodes.
 * SkyBox is simpler than SkySphere or SkyDome.
 */
class SkyBox final
{
private:
	uint32_t program;
	Texture* texture;
	uint32_t vao;
	uint32_t vbo[2];

protected:
	/**
	 * @param[in] viewProjection calculated by #getViewProjection(const mat4f&, const mat4f&)
	 */
	void render(const mat4f& viewProjection) const;
	
public:
	SkyBox();
	virtual ~SkyBox();
	
	void setTexture(Texture* texture);
	Texture* getTexture() const;
	
	void prepare();
	
	void upload() const;
	
	static mat4f getViewProjection(const mat4f& view, const mat4f& projection);

	/**
	 * @param[in] viewProjection calculated by #getViewProjection(const mat4f&, const mat4f&)
	 */
	void render(const mat4f& view, const mat4f& projection) const;
};

inline Texture* SkyBox::getTexture() const { return texture; }

}  // namespace pea
#endif  // PEA_SCENE_SKYBOX_H_
