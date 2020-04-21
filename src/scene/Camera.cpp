#include "scene/Camera.h"

#include "geometry/Sphere.h"


using namespace pea;

// UP: math use (0, 0, 1), OpenGL use (0, 1, 0)
Camera::Camera(const vec3f& position):
#if USE_MATH_XYZ
		Camera(position, position + vec3f(0, +1, 0), vec3f(0, 0, 1))
#else
		Camera(position, position + vec3f(0, 0, -1), vec3f(0, 1, 0))
#endif
{
	// math front uses positive Y axis: yaw == M_PI / 2, pitch == 0
	// game front uses negative Z axis: yaw ==-M_PI / 2, pitch == 0
}

Camera::Camera(const vec3f& position, const vec3f& target, const vec3f& up):
		position(position),
		forward(normalize(target - position)),
		worldUp(normalize(up)),
		fieldOfView(static_cast<float>(M_PI / 4)),
		aspectRatio(16.0F / 9),
		near(1.0F),
		far(1024.0F),
		speed(0.45F),
		pose(POSE_NORMAL),
		flying(false)
{
//	auto [this->pitch, this->yaw] = decomposeDirection(forward); won't work.
//	std::tie(pitch, yaw) = Sphere::decomposeOrientation(this->forward);
//	slog.i(TAG, "pitch=%f, yaw=%f", pitch, yaw);
	
	this->right = cross(this->forward, this->worldUp).normalize();
	this->up = cross(this->right, this->forward);
	projectionMatrix = perspective(fieldOfView, aspectRatio, near, far);
}
/*
Camera::~Camera()
{
	slog.v(TAG, "say bye-bye to Camera");
}
*/
void Camera::setFieldOfView(float radians)
{
	assert(0 < radians && radians < M_PI);
	this->fieldOfView = radians;
	projectionMatrix = perspective(fieldOfView, aspectRatio, near, far);
}

void Camera::setAspectRatio(float ratio)
{
	assert(0 < ratio);
	this->aspectRatio = ratio;
	projectionMatrix = perspective(fieldOfView, aspectRatio, near, far);
}

void Camera::setDepth(float near, float far)
{
	assert(0 < near && near < far);
	this->near = near;
	this->far  = far;
	
	projectionMatrix = perspective(fieldOfView, aspectRatio, near, far);
}

void Camera::setPosition(const vec3f& position)
{
	this->position = position;
}

const vec3f& Camera::getPosition() const
{
	return position;
}

vec3f Camera::getPosition(const vec3f& coordinate) const
{
	return position + 
			coordinate.x * right + 
			coordinate.y * forward + 
			coordinate.z * up;
}

void Camera::setForward(const vec3f& direction)
{
	this->forward = normalize(direction);
	this->right = cross(this->forward, worldUp);
	this->up = cross(this->right, this->forward);
}

void Camera::translate(const vec3f& offset)
{
	position += offset;
}
/*
void Camera::translateLocal(const vec3f& offset)
{
	vec4f vector(offset.x, offset.y, offset.z, 1.0f);
#if COLUMN_MAJOR
	vec4f amount = viewMatrix * vector;
#else
	vec4f amount = vector * viewMatrix;
#endif
	assert(fuzzyEqual(amount.w, 1.0f));
	translate(*reinterpret_cast<vec3f*>(&amount));
}
*/


static mat4f composeTransform(const vec3f& right, const vec3f& forward, const vec3f& up, const vec3f& position)
{
//	std::cout << "right=" << right << ", forward=" << forward << ", up=" << up << ", position=" << position << '\n'; 
/*	constexpr float epsilon = std::numeric_limits<float>::epsilon();
	assert(std::abs(right.length() - 1) <= epsilon);
	assert(std::abs(forward.length() - 1) <= epsilon);
	assert(std::abs(up.length() - 1) <= epsilon);
*/
	const vec3f& r = right, &f = forward, &u = up;
	float rw = dot(r, position);
	float fw = dot(f, position);
	float uw = dot(u, position);

#if USE_MATH_XYZ
	return mat4f(
		r.x, r.y, r.z, -rw,
		f.x, f.y, f.z, -fw,
		u.x, u.y, u.z, -uw,
		  0,   0,   0,   1);
#else
	return mat4f(
		 r.x,  r.y,  r.z, -rw,
		 u.x,  u.y,  u.z, -uw,
		-f.x, -f.y, -f.z, +fw,
		   0,    0,    0,   1);
#endif
}

mat4f Camera::getViewMatrix() const
{
//	mat4f viewMatrix = lookAt(position, position + forward, worldUp);
//	return viewMatrix;
	return composeTransform(right, forward, up, position);
}

mat4f Camera::multiply(const mat4f& view, const mat4f& projection)
{
#if COLUMN_MAJOR
	mat4f viewProjection = projection * view;
#else
	mat4f viewProjection = view * projection;
#endif
	return viewProjection;
}

static float height(float t)
{
	// ds = v0 * t + 0.5 * g * t^2;
	return 0.5f * t + 0.5f * 9.8f * (t * t);
}
/*
void handleKey(Movement direction, float offset)
{
	float ds = offset;  // speed * dt;
//	mat4f& viewMatrix = this->viewMatrix;
//	auto getViewAxis = [&viewMatrix](Axis axis) -> vec3f& { return getAxis(viewMatrix, axis); };

//	slog.v(TAG, "position: (%f, %f, %f)", position.x, position.y, position.z);
	if(direction == JUMP && pose != POSE_ABOVE)
	{
		jump_time = std::clock() / static_cast<float>(CLOCKS_PER_SEC);
		jump_height = position.z;  // TODO
	}

	if(pose == POSE_ABOVE)
	{
		float this_time = std::clock() / static_cast<float>(CLOCKS_PER_SEC);
		float dt = this_time - jump_time;
		float ds = 0.5f * dt + 0.5f * 9.8f * (dt * dt);
		if(ds > 0)
			position.z = jump_height + ds;
		else
		{
			position.z = jump_height;
			pose = POSE_NORMAL;
		}
	}

	if(direction == CROUCH)
	{
		if(pose == POSE_CROUCH)
			position -= 0.42f * worldUp;
	}
	else
		pose = POSE_NORMAL;
}
*/
void Camera::orbit(const vec3f& pivot, float pitch, float yaw, bool constrainPitch/* = true*/)
{
	vec3f direction = position - pivot;
#if 1  // high precision
	vec3d _direction(direction.x, direction.y, direction.z);
	double length = _direction.length();
	vec3d _normal = _direction / length;
	vec3f normal(_normal.x, _normal.y, _normal.z);
#else
	float length = direction.length();
	vec3f normal = direction / length;  // direction.normalize();
#endif
	
	constexpr float LIMIT = 88.0_deg;
	if(constrainPitch)  // Make sure that when pitch is out of bounds, screen doesn't get flipped.
		pitch = clamp(pitch, -LIMIT, LIMIT);
	else
		Sphere::wrap(pitch, yaw);
	
	// yaw constrains to [-pi, +pi)
#if 1
	if(yaw >= M_PI)
		yaw -= 2 * M_PI;
	else if(yaw < -M_PI)
		yaw += 2 * M_PI;
#else
	yaw = static_cast<float>(std::fmod(yaw + M_PI, 2 * M_PI) - M_PI);
#endif

	vec3f orientation = Sphere::composeOrientation(pitch, yaw);
	forward = -orientation;
	assert(std::abs(forward.length() - 1) <= std::numeric_limits<float>::epsilon());
	right = cross(forward, worldUp).normalize();
	up = cross(right, forward);  // normalize();
//	printf("Camera pitch=%f, yaw=%f, forward=(%f, %f, %f)\n", rad2deg(pitch), rad2deg(yaw), forward.x, forward.y, forward.z);
	position = pivot - length * forward;
//	slog.i(TAG, "position=((%f, %f, %f)", position.x, position.y, position.z);
}

void Camera::walk(float offset)
{
	// move along forward vector
	if(flying)
		position += forward * offset;
	else
	{
		vec3f _forward = cross(worldUp, right);
		position += _forward * offset;
	}
}

void Camera::strafe(float offset)
{
	// move along right vector
	if(flying)
		position += right * offset;
	else
	{
		vec3f _right = cross(forward, worldUp);
		position += _right * offset;
	}
}

void Camera::fly(float offset)
{
	// move along up vector
	position += (flying? up: worldUp) * offset;
}
/*
void Camera::pitch(float angle)
{
	rotate around right vector
	vec3f right(transform.a[0], transform.a[1], transform.a[2]);
	up.rotate(right, angle);
	forward.rotate(right, angle);
}

void Camera::yaw(float angle)
{
	rotate around up vector
	vec3f up2 = flying? up:vec3f(0, 1.0, 0);
	right.rotate(up2, angle);
	forward.rotate(up2, angle);
	right.rotate(up, angle);
	forward.rotate(up, angle);
}
*/
void Camera::roll(float angle)
{
	// rotate around forward vector
	if(flying)  // only applys for flying mode
	{
		right.rotate(forward, angle);
		up.rotate(forward, angle);
	}
}

std::string Camera::toString() const
{
	std::ostringstream oss;
	oss << "viewMatrix:" << '\n' << getViewMatrix() << '\n';
	oss << "projectionMatrix" << '\n' << projectionMatrix << '\n';
	return oss.str();
}

mat4f Camera::lookAt(const vec3f& position, const vec3f& target, const vec3f& up)
{
	vec3f forward = normalize(target - position);
	vec3f right = cross(forward, up).normalize();
	vec3f up_ = cross(right, forward);  // .normalize();

	return composeTransform(right, forward, up_, position);
/*
	vec4<T> v0(right.x, forward.x, up2.x, 0);
	vec4<T> v1(right.y, forward.y, up2.y, 0);
	vec4<T> v2(right.z, forward.z, up2.z, 0);
	vec4<T> v3(    -rw,       -fw,   -uw, 1);
	return mat4f(v0, v1, v2, v3);
*/
}

mat4f Camera::ortho(float left, float right, float bottom, float top, float near, float far)
{
	assert(left != right && bottom != top && near != far);
//	float width = right - left, height = top - bottom, depth = far - near;
	mat4f m(1.0F);
	m[0][0] =               2 / (right - left);
	m[3][0] = -(right + left) / (right - left);
#if USE_MATH_XYZ
	m[1][1] =               2 / (far - near);
	m[3][1] =   -(far + near) / (far - near);
	m[2][2] =               2 / (top - bottom);
	m[3][2] = -(top + bottom) / (top - bottom);
#else
	// m *= mat4f::rotateX(M_PI);  // rotate X 90 degrees counterclockwise
	m[1][1] =               2 / (top - bottom);
	m[3][1] = -(top + bottom) / (top - bottom);
	m[2][2] =              -2 / (far - near);
	m[3][2] =   -(far + near) / (far - near);
#endif
	return m;
}

mat3f Camera::ortho(float left, float right, float bottom, float top)
{
	assert(left != right && bottom != top);
	
	mat3f m(1.0F);
	m[0][0] =               2 / (right - left);
	m[2][0] = -(right + left) / (right - left);

	m[1][1] =               2 / (top - bottom);
	m[2][1] = -(top + bottom) / (top - bottom);

	return m;
}

// http://www.songho.ca/opengl/gl_projectionmatrix.html
mat4f Camera::frustum(float left, float right, float bottom, float top, float near, float far)
{
	assert(left != right && bottom != top && near != far);
	mat4f m(0.0F);

	m[0][0] =        2 * near / (right - left);
	m[2][0] =  (right + left) / (right - left);
#if USE_MATH_XYZ
	m[1][1] =   -(far + near) / (far - near);
	m[3][1] =  2 * near * far / (far - near);
	
	m[2][2] =       -2 * near / (top - bottom);
	m[3][2] = -(top + bottom) / (top - bottom);
#else
/*
	2*n/(r-l),            0,  (r+l)/(r-l),            0
	0        ,    2*n/(t-b),  (t+b)/(t-b),            0
	0        ,            0, -(f+n)/(f-n), -2*f*n/(f-n)
	0        ,            0,           -1,            0
*/
	// m *= mat4f::rotateX(M_PI);  // rotate X 90 degrees counterclockwise
	m[1][1] =        2 * near / (top - bottom);
	m[2][1] =  (top + bottom) / (top - bottom);
	
	m[2][2] =   -(far + near) / (far - near);
	m[3][2] = -2 * near * far / (far - near);
#endif
	m[2][3] = -1;
	return m;
}

mat4f Camera::perspective(float fieldOfView, float aspectRatio, float near, float far)
{
	assert(0 < fieldOfView && fieldOfView < static_cast<float>(M_PI));
	assert(0 < aspectRatio);
	assert(0 < near && 0 < far && near != far);
	double _fieldOfView = fieldOfView;
	float cot_half_fieldOfView = 1 / std::tan(_fieldOfView / 2);
/*
	float neg_depth = near - far;
	float _11 = 1 / tan_half_fieldOfView;  // reciprocal
	float _00 = _11 / aspect;
	float _22 = (far + near) / neg_depth;
	float _32 = 2 * near * far / neg_depth;
		return mat4f(
		_00,   0,   0,   0,
		  0, _11,   0,   0,
		  0,   0, _22, _32,
		  0,   0,  -1,   0);
*/
	mat4f m(0.0F);
	m[0][0] = cot_half_fieldOfView / aspectRatio;
#if USE_MATH_XYZ
	m[1][1] = (far + near) / (far - near);
	m[3][1] = 2 * near * far / (far - near);
	m[2][2] = cot_half_fieldOfView;
#else
	m[1][1] = cot_half_fieldOfView;
	m[2][2] = -(far + near) / (far - near);
	m[3][2] = -2 * near * far / (far - near);
#endif
	m[2][3] = -1;
	return m;

}
