#ifndef PEA_IO_BIOVISION_HIERARCHY_H_
#define PEA_IO_BIOVISION_HIERARCHY_H_

#include <cstdint>
#include <string>
#include <vector>

#include "math/mat4.h"
#include "math/vec3.h"
#include "scene/Bone.h"

namespace pea {

/**
 * BioVision Hierarchy
 * A BVH file has two parts, a header section which describes the hierarchy and initial pose of the 
 * skeleton; and a data section which contains the motion data.
 *
 * MAY NEED TO SUPPORT MULTIPLE ROOTS HERE! Still unsure whether multiple roots are possible?
 * location unit, rotation unit, not sure.
 * https://www.mindfiresolutions.com/BVH-biovision-hierarchy.htm
 * https://www.cs.cityu.edu.hk/~howard/Teaching/CS4185-5185-2007-SemA/Group12/BVH.html
 * https://research.cs.wisc.edu/graphics/Courses/cs-838-1999/Jeff/BVH.html
 * http://www.dcs.shef.ac.uk/intranet/research/public/resmes/CS0111.pdf
 */
class BioVisionHierarchy
{
public:
	/**
	 * Possible animation channels for which the motion data holds the values. Also known as DOF
	 * (Degree of Freedom).
	 */
	enum Channel: uint8_t
	{
		POSITION_X = 0,
		POSITION_Y,
		POSITION_Z,
		
		ROTATION_X,
		ROTATION_Y,
		ROTATION_Z,
		
		CHANNEL_COUNT,
	};

	struct Joint
	{
		/**
		 * joint's name, end joint (delimited by the keyword End Site)'s name is empty.
		 */
		std::string name;
		/**
		 * Translation of the origin of the joint with respect to its parent’s origin (or globally 
		 * inthe case of the root joint) along the x, y and z-axis respectively. 
		 */
		vec3f offset;
		
		std::vector<Channel> channels;
		int32_t parent;
	};
	
	int32_t frameCount;
	float frameTime;
	int32_t valueCountPerFrame;
	std::vector<Joint> joints;
	std::vector<float> values;
	
public:
	BioVisionHierarchy();
	~BioVisionHierarchy() = default;
	
	/**
	 * Note that BVH content needs to be cleared before another loading.
	 * @param[in] path file path.
	 * @return true if loading data successfully, otherwise false.
	 */
	bool load(const std::string& path);
	
	/**
	 * @param[in] positionFactor Position scaled by this value, must be positive. Assume lengths are
	                             in meters, so this value is 0.01 when reading a .BVH file with 
	                             centimeter unit.
	 * @param[in] rotationFactor Rotation scaled by this value. Assume angles are in radians, so  
	 *                           this value is M_PI / 180 when reading a .BVH file with degree unit.
	 */
	void scale(float positionFactor, float rotationFactor = 1.0);
	
	/**
	 * @param[in] path the path to be saved.
	 * @param[in] precision float point number precision, default to 6.
	 * @return true if archived successfully, otherwise false.
	 */
	bool save(const std::string& path, int32_t precision = 6) const;
	
	/**
	 * clear BVH data
	 * @param[in] keepHierarchy set it to true if you want to keep hierarchy data, wipe motion data,
	 *            otherwise clear all the data.
	 */
	void clear(bool keepHierarchy);
	
	bool hasHierarchyData() const;
	bool hasMotionData() const;
	int32_t getFrameCount() const;
	
//	mat4f calculateLocalTransform(int32_t boneIndex, int32_t frameIndex) const;
	
//	mat4f calculateGlobalTransform(int32_t boneIndex, int32_t frame) const;

	/**
	 * @param[out] transforms Global transform of each joint.
	 * @param[in] frameIndex Frame index, starts from 0.
	 */
	void calculateGlobalTransforms(std::vector<mat4f>& transforms, int32_t frameIndex) const;
	
//	void exportRestPose(std::vector<Bone>& bones) const;
	
	/**
	 * @param[out] bones
	 * @param[in]  frameIndex negative for rest pose, 0 for the first frame.
	 */
	void exportFramePose(std::vector<Bone>& bones, int32_t frameIndex) const;
protected:
	
	void openHierarchy();
	void closeHierarchy();
	
	// hierarchy part
	void addJoint();
	
	// motion data
	
	void addFrameData();
};

inline int32_t BioVisionHierarchy::getFrameCount() const { return frameCount; }

}  // namespace pea
#endif  // PEA_IO_BIOVISION_HIERARCHY_H_
