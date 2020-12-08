#include <thread>

#include "geometry/Grid.h"
#include "geometry/Teapot.h"
#include "geometry/Torus.h"
#include "graphics/Image_PNG.h"
#include "io/FileSystem.h"
#include "opengl/glLoader.h"
#include "opengl/Program.h"
#include "opengl/Shader.h"
#include "opengl/ShaderFactory.h"
#include "scene/Camera.h"
#include "scene/Mesh.h"
#include "util/Log.h"

#include <GLFW/glfw3.h>

using namespace pea;

static const char* title = "Tessellation";
static const char* TAG = title;

// global variables
int32_t subdivision = 3;
bool wireframe = false;
bool solid = true;
float theta = -M_PI / 2;
bool paused = false;
constexpr float radius = 3.0F, height = 5.0;
const vec3f target(0, 0, 0);
Camera camera(vec3f(0, -radius, height), target, vec3f(0, 0, 1));


void onWindowResize(GLFWwindow* window, int width, int height)
{
	(void)window;  // suppress unused parameter warning
	glViewport(0, 0, width, height);
	
	float aspectRatio = static_cast<float>(width) / height;
	camera.setAspectRatio(aspectRatio);
}

static void onKey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	std::ignore = scancode;
	std::ignore = mods;
	if(action != GLFW_PRESS)
		return;
	
	auto updateTitle = [&window]()
	{
		std::ostringstream oss;
		oss << title << ' ' << '(' << subdivision << ')';
		glfwSetWindowTitle(window, oss.str().c_str());
	};
	
	switch(key)
	{
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		break;
	
	case GLFW_KEY_W:
		wireframe = !wireframe;
		break;
	
	case GLFW_KEY_S:
		solid = !solid;
		break;
	
	case GLFW_KEY_1:
	case GLFW_KEY_2:
	case GLFW_KEY_3:
	case GLFW_KEY_4:
	case GLFW_KEY_5:
	case GLFW_KEY_6:
	case GLFW_KEY_7:
	case GLFW_KEY_8:
	case GLFW_KEY_9:
		subdivision = key - GLFW_KEY_0;
		updateTitle();
		break;
	
	case GLFW_KEY_KP_ADD:
	case GLFW_KEY_KP_SUBTRACT:
	{
		bool isControlDown = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
		glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
		if(isControlDown)
		{
			if(key == GLFW_KEY_KP_ADD)
				++subdivision;
			else
			{
				--subdivision;
				if(subdivision < 1)
					subdivision = 1;
			}
			updateTitle();
		}
		
		break;
	}
	
	case GLFW_KEY_SPACE:
		paused = !paused;
		break;
	}
}

std::unique_ptr<Mesh> createMesh(const Teapot& teapot, bool wireframe)
{
	Primitive primitive = wireframe? Primitive::LINES: Primitive::TRIANGLES;
	std::vector<vec3f> vertices = teapot.getVertexData();
	std::vector<uint32_t> indices = teapot.getIndexData(primitive);
	Mesh::Builder builder(vertices);
	if(!wireframe)
	{
		std::vector<vec3f> normals = teapot.getNormalData();
		builder.setNormal(normals);
	}
	builder.setIndex(indices);
	
	std::unique_ptr<Mesh> mesh = builder.build();
	mesh->prepare(primitive);
	mesh->upload();
	return mesh;
}

/*
Instructions:
  Esc           Exit this program
  W             Display/Hide wireframe model
  S             Display/Hide solid model
  Space         Play/Pause animation
  1~9           Set subdivision to number 1~9
  Ctrl + NUM -  Decrease subdivision
  Ctrl + NUM +  Increase subdivision
*/
int main()
{
	if(!glfwInit())  // needs to call glfwTerminate() after glfwInit successfully.
	{
		slog.e(TAG, "GLFW failed to initialize");
		return -1;
	}
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// open a window and create its OpenGL context
	int32_t windowWidth = 1280;
	int32_t windowHeight = 720;
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, title, nullptr/* monitor */, nullptr/* share */);
	if(!window)
	{
		slog.e(TAG, "Failed to open GLFW window. Not OpenGL 3.3 compatible");
		glfwTerminate();
		return -2;
	}
	
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, onWindowResize);
	glfwSetKeyCallback(window, onKey);
	
	loadGL();
	GL::enableDebugMessage();
	
	// global OpenGL state
	glEnable(GL_DEPTH_TEST);
	
	camera.setDepthRange(1.0, 8);
	Program wireframeProgram(ShaderFactory::VERT_M_VP, ShaderFactory::FRAG_UNIFORM_COLOR);
	Program solidProgram(ShaderFactory::VERT_M_VP_NORMAL, ShaderFactory::FRAG_MATCAP);
	
	Program normalProgram(ShaderFactory::VERT_M_VP_NORMAL2, ShaderFactory::GEOM_NORMAL, ShaderFactory::FRAG_UNIFORM_COLOR);

	Teapot teapot(3);  // 4x4 patch's subdivision value is 3
	std::unique_ptr<Mesh> wireframeTeapot = createMesh(teapot, true/*wireframe*/);
	wireframeTeapot->setProgram(wireframeProgram.getName());
	vec4f lineColor(0.0, 1.0, 0.0, 1.0);  // green
	wireframeTeapot->setUniform(Shader::UNIFORM_VEC_UNIFORM_COLOR, Type::VEC4F, &lineColor);
	
	std::unique_ptr<Mesh> solidTeapot = createMesh(teapot, false/*wireframe*/);
	solidTeapot->setProgram(solidProgram.getName());
	
	Texture texture(GL_TEXTURE_2D);
	std::string path = FileSystem::getRelativePath("res/image/matcap/metal.jpg");
	Texture::Parameter parameter(10);
	texture.load(path, parameter);
	solidTeapot->setTexture(texture, 0);
	
	while(!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		if(paused && subdivision == teapot.getSubdivision())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(16));
			continue;
		}
		
		theta += 1_deg;
		vec3f position(radius * std::cos(theta), radius * std::sin(theta), height);
		
		camera.setPosition(position);
		camera.setForward((target - position).normalize());
		
		mat4f viewMatrix = camera.getViewMatrix();
		mat4f projectionMatrix = camera.getProjectionMatrix();
		mat4f viewProjection = Camera::multiply(viewMatrix, projectionMatrix);
		
		if(subdivision != teapot.getSubdivision())
		{
			// benchmark time cost, start thread if too costly.
			std::time_t startTime = std::clock();
			
			teapot = Teapot(subdivision);
			solidTeapot = createMesh(teapot, false/*wireframe*/);
			solidTeapot->setProgram(solidProgram.getName());
			solidTeapot->setTexture(texture, 0);
			
			std::time_t endTime = std::clock();
			double elapsedTime = static_cast<double>(endTime - startTime) / CLOCKS_PER_SEC;
			slog.d(TAG, "dynamically create teapot, time cost=%lfs", elapsedTime);
		}
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if(solid)
			solidTeapot->render(viewProjection);
		if(wireframe)
			wireframeTeapot->render(viewProjection);
	
		glfwSwapBuffers(window);
	}
	
	glfwTerminate();
	return 0;
}
