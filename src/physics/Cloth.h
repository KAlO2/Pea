#ifndef PEA_PHYSICS_CLOTH_H_
#define PEA_PHYSICS_CLOTH_H_

#include "io/Model.h"

namespace pea {

class Particle;
class Constraint;

/**
 * Some useful links are listed here:
 * https://graphics.stanford.edu/courses/cs468-02-winter/Papers/Rigidcloth.pdf
 * http://davis.wpi.edu/~matt/courses/cloth/
 * Stable but Responsive Cloth http://graphics.snu.ac.kr/~kjchoi/publication/cloth.pdf
 * http://blender.stackexchange.com/questions/32043/how-does-the-cloth-stiffness-scaling-option-work
 * https://docs.blender.org/manual/en/latest/physics/cloth/introduction.html
 */
class Cloth
{
public:
	enum SpringType: uint8_t
	{
		TENSION     = 0,
		COMPRESSION = 1,
		SHEAR       = 2,
		BENDING     = 3,
//		SpringTypeCount
	};

private:
	static constexpr uint8_t SpringTypeCount = 4;
	
	const Model& model;
	std::vector<Particle> particles;
	std::vector<Constraint*> constraints;
	std::string pinnedGroupName;

	vec3f windForce;

private:
	void calculateForce();
	
public:
	/**
	 * @param[in] model
	 * @param[in] mass cloth mass, in Kilogram.
	 */
	Cloth(const Model& model, float mass);
	
	~Cloth();
	
	void setStiffness(float stiffness);
	void setStiffness(const float stiffness[SpringTypeCount]);
	
	void setDamping(float damping);
	void setDamping(const float damping[SpringTypeCount]);
	
	/**
	 * Pinning makes particle immovable, namely particle is not influenced by external forces.
	 * @param[in] vertex group name, empty or nonexistent group reset all particles to movable.
	 */
	bool pinGroup(const std::string& name);
	
	/**
	 * @return pinned vertex group name, return empty if not any group.
	 */
	std::string getPinnedGroup() const;
	
	void step(float dt);
	
	/**
	 * Baking means precalculating physics simulations such as Cloth or Rigid Body. You can saves 
	 * them to a file, which then loads when rendering or playing back the animation so that the 
	 * physics don't have to be recalculated every time.
	 */
	void exportVertextexData(std::vector<vec3f>& vertices) const;
};

}  // namespace pea
#endif  // PEA_PHYSICS_CLOTH_H_
