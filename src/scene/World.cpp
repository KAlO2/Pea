#include "scene/World.h"

#include <algorithm>

#include "geometry/Sphere.h"
#include "io/FileSystem.h"
#include "opengl/GL.h"
#include "opengl/glLoader.h"
#include "opengl/Shader.h"
#include "scene/Mesh.h"
#include "scene/Floor.h"
#include "util/Log.h"

#include <GLFW/glfw3.h>

using namespace pea;

static const char* TAG = "World";

// https://en.wikipedia.org/wiki/Gravity
static constexpr float GRAVITY_ACCELERATION = 9.80665F;  // m/s^2

static constexpr float STAND_HEIGHT = 1.66;
static constexpr float CROUCH_HEIGHT = 0.68;
static constexpr float CROUCH_SPEED = 0.80;
static_assert(0 < CROUCH_HEIGHT && CROUCH_HEIGHT < STAND_HEIGHT, "the height gets lower after one crouches");

World::World(int32_t width, int32_t height, const char* title):
		Window(width, height, title),
		camera(vec3f(0, -3, STAND_HEIGHT), vec3f(0.0f, 0.0f, 0.0f), vec3f(0.0f, 0.0f, 1.0f)),
		floor(nullptr),
		velocity(0),
		humanState(HumanState::ON_THE_GROUND)
{

}

World::~World()
{
	for(Mesh* object: objects)
		delete object;
	objects.clear();
	delete floor;
}

void World::setState(HumanState state)
{
	if(humanState == HumanState::ON_THE_GROUND)
	{
		if(state == HumanState::IN_THE_AIR)
			velocity = GRAVITY_ACCELERATION * 0.30;  // v = a*t;
		else if(state == HumanState::CROUCH)
			velocity = -0.8;  // can be tweaked.
		
		humanState = state;
	}
}

void World::stand(float dt)
{
	vec3f position = camera.getPosition();
	float z = position.z;
	if(humanState == HumanState::IN_THE_AIR)
	{
		// s1 = v0 * t - 1/2 * g * t^2
		constexpr float a = -9.8;  // acceleration of gravity
		float v1 = velocity + a * dt;
		float ds = (velocity + v1) * dt / 2;
		velocity = v1;
		z += ds;
		if(z < STAND_HEIGHT)
		{
			velocity = 0;
			z = STAND_HEIGHT;
			humanState = HumanState::ON_THE_GROUND;
		}
	}
	else if(humanState == HumanState::CROUCH)
	{
		z += velocity * dt;
		if(z <= CROUCH_HEIGHT)
		{
			z = CROUCH_HEIGHT;
			velocity = 0;
		}
		else if(z >= STAND_HEIGHT)
		{
			z = STAND_HEIGHT;
			velocity = 0;
			humanState = HumanState::ON_THE_GROUND;
		}
	}
	
	if(z != position.z)
	{
		position.z = z;
		camera.setPosition(position);
	}
}

void World::onKey(int32_t key, int32_t scanCode, int32_t action, int32_t modifierKeys)
{
	assert(window != nullptr);
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	
	if(glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
		setLayoutVisibility(Visibility::VISIBLE);
	else
		setLayoutVisibility(Visibility::INVISIBLE);
	
	float speed = 2.0f;
	float offset = speed * deltaTime;
	// LEFT and RIGHT keys can be pressed simultaneously, so if / else if style is not advisable.
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		camera.walk(offset);
	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		camera.walk(-offset);
	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		camera.strafe(-offset);
	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		camera.strafe(offset);
	
	// space for jumping
	if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		setState(HumanState::IN_THE_AIR);
	
	// Key C must always be pressed to stay in crouching state
	if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
	{
		velocity = -CROUCH_SPEED;
		humanState = HumanState::CROUCH;
	}
	else if(humanState == HumanState::CROUCH)
		velocity = +CROUCH_SPEED;
}

void World::onWindowResize(int32_t width, int32_t height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
	camera.setAspectRatio(static_cast<float>(width) / height);
}

void World::onWindowFocusChanged(bool focused)
{
	slog.v(TAG, "window %ss focus", (focused? "gain": "lose"));
	// disable cursor, hide HUD (head-up display)
	setCursorVisibility(focused? Visibility::GONE: Visibility::VISIBLE);
	setLayoutVisibility(Visibility::INVISIBLE);
//	setCursorPosition(vec2i(width >> 1, height >> 1));  // center of the window
}

void World::onMouseMove(float dx, float dy)
{
//	slog.i(TAG, "dx=%f, dy=%f", dx, dy);
	vec2i windowSize = getSize();
	float mouseSensitivity = M_PI / windowSize.height;
	auto [pitch, yaw] = Sphere::decomposeOrientation(camera.getForward());
	float pitchAngle = pitch - dy * mouseSensitivity;
	float yawAngle = yaw - dx * mouseSensitivity;
	camera.orbit(pitchAngle, yawAngle);
}

void World::onMouseScroll(float dx, float dy)
{
	std::ignore = dx;
	float angle = dy / 180.0F;
	
	// upperLimit and upperLimit can be tweaked.
	constexpr float lowerLimit = M_PI / 18;
	constexpr float upperLimit = M_PI * 2 / 3;
	float fieldOfView = camera.getFieldOfView();
	fieldOfView = clamp(fieldOfView + angle, lowerLimit, upperLimit);
	
	camera.setFieldOfView(fieldOfView);
}

void World::setTextureSky(Texture* texture)
{
	skybox.setTexture(texture);
}

Texture* World::getTextureSky() const
{
	return skybox.getTexture();
}

void World::prepare()
{
	// Global OpenGL states
//	glClearColor(0.0F, 0.0F, 0.0F, 0.0F);
	glEnable(GL_MULTISAMPLE);
	
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);  // default to GL_CCW
	
	glEnable(GL_BLEND);  // enable alpha blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // src.rgb * src.a + dst.rgb * (1 - src.a)
	
	vec3f center(0, 0, 0);
	float extend = 100.0F;
	floor = new(std::nothrow) Floor(center, extend, extend);
	if(floor)
	{
		floor->prepare();
		floor->uploadVertexData();
		
		Texture texture(GL_TEXTURE_2D);
		Texture::Parameter parameter(10);
		parameter.setMapMode(GL_MIRRORED_REPEAT);
		texture.load(FileSystem::getRelativePath("res/image/brick.jpg"));
		texture.setParameter(parameter);
		floor->setTexture(std::move(texture));
	}
	
	skybox.prepare();
	skybox.upload();
}

void World::updateViewPositionUniform()
{
	vec3f viewPosition = camera.getPosition();
	for(Mesh* object: objects)
	{
		constexpr int32_t location = Shader::UNIFORM_VEC_VIEW_POSITION;
#if 1
		if(object->hasUniform(location))
			object->setUniform(location, pea::Type::VEC3F, &viewPosition);
#else
		Uniform uniform;
		if(object->hasUniform(location, &uniform))
			object->updateUniform(uniform, &viewPosition);
#endif
	}
}

void World::render()
{
	mat4f view = camera.getViewMatrix();
	mat4f projection = camera.getProjectionMatrix();
	
	// update view position if necessary
	updateViewPositionUniform();
	
	render(view, projection);
}

void World::render(const mat4f& view, const mat4f& projection) const
{
	glEnable(GL_DEPTH_TEST);
	
	mat4f viewProjection = Camera::multiply(view, projection);
	
	// clear default frame buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(floor)
		floor->render(viewProjection);
	
	for(const Object* object: objects)
		object->render(viewProjection);
	
	// skybox render last. Early fragment test.
	skybox.render(view, projection);
}

void World::update(float dt)
{
	deltaTime = dt;
	stand(dt);  // free fall if man is in the air, stand when crouch key is not pressed.
	
	for(Object* object: objects)
		object->update(dt);
}

void World::addObject(Mesh* object)
{
	if(object)
		objects.push_back(object);
}

Mesh* World::findObject(const std::string& name)
{
	for(Mesh* object: objects)
		if(object->getName() == name)
			return object;
	
	return nullptr;
}

Mesh* World::findObject(uint32_t id)
{
	for(Mesh* object: objects)
		if(object->getId() == id)
			return object;
	
	return nullptr;
}

bool World::removeObject(const Mesh* object)
{
	auto it = std::find(objects.begin(), objects.end(), object);
	if(it == objects.end())
		return false;
	
	objects.erase(it);
	return true;
}
