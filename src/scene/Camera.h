#ifndef PEA_SCENE_CAMERA_H_
#define PEA_SCENE_CAMERA_H_

#include <string>

#include "math/mat3.h"
#include "math/mat4.h"

#ifdef _WIN32
/*
Damn Microsoft! 
<windows.h> header file includes <windef.h>, which unconditionally defines the following macros:
#define far
#define near
*/
#ifdef near
#	undef near
#endif
#ifdef far
#	undef far
#endif
#endif

namespace pea {

/*
	   |
	above
	   |
	-----on------ (water surface)
	below         (about 1 meter)
	- - - - - -   (virtual Line, if player does not dive or uses magic to walk on Water, he will "walk" on this line. Dive will become to "swim")
	in            (under 1 meter of water surface)
	\___________/  bottom of water

*/


class Camera final
{
public:
	// Defines several possible options for camera movement. Used as abstraction to
	// stay away from window-system specific input methods
/*	enum Movement
	{
		LEFT,     // -x strafe left
		RIGHT,    // +x strafe right
		BACKWARD, // -y move backward
		FORWARD,  // +y move forward
		CROUCH,   // -z
		JUMP      // +z
	};
*/
	enum class Axis: uint8_t
	{
/*
		AXIS_X = 0,
		AXIS_Y = 1,
		AXIS_Z = 2,
*/
		RIGHT = 0,// X
#if USE_MATH_XYZ
		FORWARD,  // Y
		UP,       // Z
#else
		UP,       // Y
		BACKWARD, // Z
#endif
//		POSITION = 3,
	};
private:
	// TODO need a tri-gate boolean???
	enum HumanBodyPose
	{
		POSE_NORMAL,
		POSE_CROUCH,
		POSE_ABOVE,
	};

	static const float ROTATION_STEP;
	
//	mat4f viewMatrix;
	mat4f projectionMatrix;

	vec3f position;
	vec3f right;
	vec3f forward;
	vec3f up;
	const vec3f worldUp;
	
	float fieldOfView;  ///< in radians, widens camera's horizon when increase.
	float aspectRatio;  ///<  determines the field of view in the x direction
	float near;    ///< distance between the viewer and the near clipping plane
	float far;     ///< distance between the viewer and the far clipping plane

	// Eular angles
//	float yaw = 0.0f;  // (-pi, pi]
//	float pitch = 0.0f;  // (-pi/2, pi/2)

	mat4f projection;
	mat4f orthogonal;

	float speed;


//	vec3f velocity;
	HumanBodyPose pose;
	float jump_time = 0.0f;
	float jump_height = 0.0f;
	bool flying;   ///< move on the ground or fly in the air?

	bool orthographic;

private:
	static const vec3f& getAxis(const mat4f& transform, Axis axis);
	static       vec3f& getAxis(mat4f& transform, Axis axis);
	
public:
	
	/**
	 * Define a viewing transformation in terms of an eye point, a center of view, and
	 * an up vector.
	 *
	 * @param[in] eye Position of the camera
	 * @param[in] target Position where the camera is looking at
	 * @param[in] up Normalized up vector, how the camera is oriented. Typically (0, 1, 0)
	 */
	static mat4f lookAt(const vec3f& eye, const vec3f& target, const vec3f& up);
	
	static mat4f frustum(float left, float right, float bottom, float top, float near, float far);
	
	/**
	 * This creates a transformation that produces a parallel projection.
	 * x maps from [min, max] to [-1, 1], that's x' = 2*(x - min)/(max -min) - 1;
	 * x' = 2*x/(max -min) - (min + max)/(max - min);
	 *
	 * @param[in] left   Specify the coordinates for the left vertical clipping planes.
	 * @param[in] right  Specify the coordinates for the right vertical clipping planes.
	 * @param[in] bottom Specify the coordinates for the bottom horizontal clipping planes.
	 * @param[in] top    Specify the coordinates for the top horizontal clipping planes.
	 * @param[in] near   Specify the distances to the nearer depth clipping planes.
	 * @param[in] far    Specify the distances to the nearer and farther depth clipping planes.
	 */
	static mat4f ortho(float left, float right, float bottom, float top, float near, float far);
	
	static mat3f ortho(float left, float right, float bottom, float top);
	
	/**
	 * Set up a perspective projection matrix
	 * see {@link https://en.wikibooks.org/wiki/GLSL_Programming/Vertex_Transformations}
	 * [d/a	0	0	0]
	 * [0	d	0	0]
	 * [0	0	(n+f)/(n-f)	2*n*f/(n-f)]
	 * [0	0	-1	0]
	 * with d = 1/tan(fov/2)
	 *
	 * @param[in] fieldOfView Specifies the field of view angle, in randians, in the Y direction.
	 * @param[in] aspectRatio Specifies the aspect ration that determines the field of view in the x
	 *                   direction. The aspect ratio is the ratio of x (width) to y (height).
	 * @param[in] near   Specifies the distance from the viewer to the near clipping plane
	 *                   (always positive).
	 * @param[in] far    Specifies the distance from the viewer to the far clipping plane
	 *                   (always positive).
	 */
	static mat4f perspective(float fieldOfView, float aspectRatio, float near, float far);
	
public:
	/**
	 * Construct a Camera looking at +Y for math, -Z for OpenGL
	 * @param[in] position position of the camera.
	 */
	Camera(const vec3f& position);
	
	/**
	 * Setup camera's position and rotation.
	 * @param[in] position position of the camera.
	 * @param[in] target   point acts as the center of the scene.
	 * @param[in] up       The up vector must not be parallel to the line of sight from the position
	 *                     to the target.
	 */
	Camera(const vec3f& position, const vec3f& target, const vec3f& up);

	~Camera() = default;

	void setFlying(bool flying);
	bool isFlying() const;
	
	const vec3f& getUp() const;
	
	void  setFieldOfView(float radians);
	float getFieldOfView() const;
	
	void  setAspectRatio(float ratio);
	float getAspectRatio() const;
	
	void  setDepthRange(float near, float far);
	float getNear() const;
	float getFar() const;

	mat4f getViewMatrix() const;
	const mat4f& getProjectionMatrix() const;
	
	/**
	 * multiply view matrix and projection matrix
	 */
	static mat4f multiply(const mat4f& view, const mat4f& projection);
	
	/**
	 * Translate world-space coordinate to screen-space coordinate, z = 0
	 */
	vec3f project(const vec3f& point) const;

	void         setPosition(const vec3f& position);
	const vec3f& getPosition() const;

	/**
	 * get position in camera space
	 */
	vec3f getPosition(const vec3f& coordinate) const;
	
	void         setForward(const vec3f& direction);
	const vec3f& getForward() const;
	
	/**
	 * translate the camera's position in world space, namely in global coordinate system.
	 */
	void translate(const vec3f& offset);

	/**
	 * translate the camera's position with respect to its orientation, namely in local coordinate system.
	 */
	void translateLocal(const vec3f& offset);
	
	/**
	 * orient around an object
	 * @param[in] pivot object's position.
	 * @param[in] pitch rotate around X axis.
	 * @param[in] yaw rotate around Z axis. positive value turns left.
	 * @param[in] constrainPitch pitch does not exceed (-90, 90) degrees.
	 */
	void orbit(const vec3f& pivot, float pitch, float yaw, bool constrainPitch = true);
	
	/**
	 * orient around self
	 */
	void orbit(float pitch, float yaw, bool constrainPitch = true);
	
	// Translation
	void walk(float offset);    // walk/surge, moving forward and backward
	void strafe(float offset);  // strafe/sway, moving left and right
	void fly(float offset);     // fly/heave, moving up and down

	// Rotation
//	void pitch(float angle); // tilts forward and backward
//	void yaw(float angle);   // swivels left and right
	void roll(float angle);  // pivots side to side

	std::string toString() const;
};

inline void Camera::setFlying(bool flying) { this->flying = flying; }
inline bool Camera::isFlying() const       { return flying;         }

inline const vec3f& Camera::getForward() const { return forward; }
inline const vec3f& Camera::getUp()    const { return up;          }

inline float Camera::getFieldOfView() const { return fieldOfView; }
inline float Camera::getAspectRatio() const { return aspectRatio; }
inline float Camera::getNear()        const { return near;        }
inline float Camera::getFar()         const { return far;         }

inline const mat4f& Camera::getProjectionMatrix() const { return projectionMatrix; }
	
inline const vec3f& Camera::getAxis(const mat4f& transform, Axis axis)
{
	uint8_t index = static_cast<uint8_t>(axis);
	const vec4f& major = transform[index];
	return *reinterpret_cast<const vec3f*>(&major);
}

inline vec3f& Camera::getAxis(mat4f& transform, Axis axis)
{
	uint8_t index = static_cast<uint8_t>(axis);
	vec4f& major = transform[index];
	return *reinterpret_cast<vec3f*>(&major);
}

}  // namespace pea
#endif  // PEA_SCENE_CAMERA_H_
