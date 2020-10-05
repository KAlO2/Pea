#ifndef PEA_SCENE_BONE_H_
#define PEA_SCENE_BONE_H_

#include "math/Transform.h"
#include "scene/Object.h"

namespace pea {

class VertexWeight
{
public:
	int32_t index;  ///< index of the vertex influenced by a bone
	float weight;   ///< range [0, 1], and weight of one vertex from all bones amounts to 1.

public:
	VertexWeight() = default;
	VertexWeight(int32_t index, float weight): index(index), weight(weight) {}
};

// source/blender/draw/engines/overlay/shaders/armature_shape_outline_vert.glsl  solid wired outline
// draw/intern/draw_cache.c
// Automatic Rigging and Animation of 3D Characters  http://people.csail.mit.edu/ibaran/papers/2007-SIGGRAPH-Pinocchio.pdf
class Bone final
{
public:
	std::string name;  ///< The name of the bone.
	
	/**
	 * Matrix that transforms from bone space to mesh space in bind pose.
	 * 
	 * This matrix describes the position of the mesh in the local space of this bone when the 
	 * skeleton was bound. Thus it can be used directly to determine a desired vertex position,
	 * given the world-space transform of the bone when animated, and the position of the vertex in 
	 * mesh space.
	 * 
	 * It is sometimes called an inverse-bind matrix, or inverse bind pose matrix.
	 */
	mat4f rest;
	
	mat4f local;  ///< local transformation with respect to parent.
	mat4f global;  ///< global transformation
//	std::vector<VertexWeight> weights;  //< vertex group that will be a
	vec3f head, tail;  // vec3 position + float radius
	int32_t parent;
	
public:
	/**
	 * construct a bone at origion, and points to +Z axis.
	 */
	Bone();
	~Bone() = default;
	
	float length() const;
	
	void reset();
	
//	mat4f getLocalTransform() const;
//	mat4f getGlobalTransform() const;
//	void render(const mat4f& viewProjection) const override;
};

}  // namespace pea
#endif  // PEA_SCENE_BONE_H_
