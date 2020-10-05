#include <cinttypes>

#include "opengl/QuadRenderer.h"
#include "opengl/Texture.h"
#include "graphics/Image_PNG.h"
#include "util/utility.h"
#include "io/FileSystem.h"

#include "common.h"

using namespace pea;


static const char* TAG = "ripple";

static float weight = 0.5F;

void onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	std::ignore = scancode;
	std::ignore = action;
	std::ignore = mods;
	
	switch(key)
	{
	case GLFW_KEY_ESCAPE:
		if(action == GLFW_RELEASE)
			glfwSetWindowShouldClose(window, true);
		break;
	
	case GLFW_KEY_LEFT:
	case GLFW_KEY_RIGHT:
		if(action == GLFW_PRESS)
		{
			bool isControlPressed = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS
					|| glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
			float step = isControlPressed? 0.05F: 0.01F;
			if(key == GLFW_KEY_LEFT)
				weight -= step;
			else
				weight += step;
			weight = clamp(weight, 0.0F, 1.0F);
			slog.d(TAG, "control=%d, left weight=%.3f", static_cast<int>(isControlPressed), weight);
		}
		break;
	}
}

Program createProgram()
{
	std::string vertexShaderSource = ShaderFactory::VERSION + R""(
layout(location = 0) in vec3 position;
layout(location = 3) in vec2 texcoord;

out vec2 _texcoord;

void main()
{
	gl_Position = vec4(position, 1.0);
	_texcoord = texcoord;
}
)"";

	std::string fragmentShaderSource = ShaderFactory::VERSION + R""(
layout(location =10) uniform sampler2D texture0;
layout(location = 9) uniform float time;

in vec2 _texcoord;

// sinc(x) = sin(x) / x;
float ripple(float distance)
{
	if(distance > 1E-4)
	{
		float d = 1.0 / (1.0 + 200.0 * distance * distance); 
		return sin(d - time) / d;
	}
	return 1.0;
}

float wave(vec2 position, vec2 center, float frequency, float numWaves, float t)
{
	const float pi = 3.14159265358979;
	float d = length(position - center);
//	d = log(1.0 + exp(d));
	float amplitude = 1.0 / (1.0 + 2000.0 * d * d);
	float f = frequency * t - numWaves * d;
	return amplitude * sin(2.0 * pi * f);
}

// This height map combines a couple of waves
float height(vec2 position, float t)
{
	// Params = (wave frequency in Hz, number of waves per unit distance)
	vec2 params = vec2(2.5, 20.0);
	
	float w;
	w  = wave(position, vec2(0.2, 0.2), params.x, params.y, t);
	w += wave(position, vec2(0.9, 0.7), params.x, params.y, t);
	return w;
}

vec2 perturb(vec2 position, float time)
{
	const float dx = 0.01, dy = dx;
	float h2 = height(position + vec2(0, dy), time);
	float h0 = height(position, time), h1 = height(position + vec2(dx, 0), time);
	// vx = vec3(dx, 0, h1 - h0); vy = vec3(0, dy, h2 - h0);
	// | i   j     k   |
	// |dx   0  h1 - h0|
	// | 0  dy  h2 - h0|
	// and normal in position is dot(vx, vy) = vec3(-dy*(h1 - h0), -dx*(h2 - h0), dx * dy);
	// = vec3(-(h1 - h0) / dx, -(h2 - h0) / dy, 1);
//	vec3 normal = vec3((h0 - h1) / dx, (h0 - h2) /dy, 1.0);
//	return normalize(normal);
//	return vec2((h1 - h0) / dx, (h2 - h0) / dy);
	return vec2(h1 - h0, h2 - h0);
}

void main()
{
	vec2 texcoord = _texcoord + perturb(_texcoord, time);
	gl_FragColor = texture(texture0, texcoord);
}
)"";
	
	uint32_t vertexShader = ShaderFactory::loadShader(Shader::VERTEX_SHADER, vertexShaderSource);
	uint32_t fragmentShader = ShaderFactory::loadShader(Shader::FRAGMENT_SHADER, fragmentShaderSource);
	assert(vertexShader != 0 && fragmentShader != 0);
	uint32_t program = Program::createProgram(2, vertexShader, fragmentShader);
	slog.i(TAG, "create program %" PRIu32, program);
	
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	
	return Program(program);
}

int main(int argc, char* argv[])
{
	const char* usage = R""(
This program demonstrates water ripple effect.
strike LEFT/RIGHT key to decrease/increate blending weight,
combined with Ctrl key, you get bigger step.
)"";
	std::ignore = argc;  // to suppress warning: unused parameter ‘argc’
	printf("%s - %s", argv[0], usage);
	
	if(!glfwInit())  // needs to call glfwTerminate() after glfwInit successfully. 
	{
		slog.e(TAG, "GLFW failed to initialize");
		return -1;
	}
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// open a window and create its OpenGL context
	int32_t windowWidth = 640;
	int32_t windowHeight = 480;
	const char* windowTitle = "Ripple";
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, nullptr/* monitor */, nullptr/* share */);
	if(!window)
	{
		slog.e(TAG, "Failed to open GLFW window. Not OpenGL 3.3 compatible");
		glfwTerminate();
		return -2;
	}
	
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, onWindowResize);
	glfwSetKeyCallback(window, onKeyEvent);

	loadGL();
	GL::enableDebugMessage();

	// build and compile our shader program
	Program program = createProgram();
	
	QuadRenderer quadRenderer;
	quadRenderer.prepare();
	quadRenderer.upload();
	
	Texture texture0;
	Texture::Parameter parameter(1);
	glActiveTexture(GL_TEXTURE0);
	texture0.load(FileSystem::getRelativePath("res/image/lena_std.jpg"), parameter);

	quadRenderer.setTexture(&texture0);
	quadRenderer.setProgram(program.getName());
	
	program.use();
//	program.printActiveVariables();
	mat4f model(1.0);
//	Program::setUniform(Shader::UNIFORM_MAT_MODEL, model);
/*
	vec3f eye(0, -2, 2), target(0, 0, 0), up(0, 0, 1); 
	Camera camera(eye, target, up);
*/
	int64_t startTimeUs = getCurrentTime();
//	glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
	while(!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT);
		
		int64_t currentTimeUs = getCurrentTime();
		float time = static_cast<float>(currentTimeUs - startTimeUs) * 1E-3;
/*
		mat4f viewMatrix = camera.getViewMatrix();
		mat4f projectionMatrix = camera.getProjectionMatrix();
		mat4f viewProjection = Camera::multiply(viewMatrix, projectionMatrix);
*/
		Program::setUniform(Shader::UNIFORM_FLT_TIME, time);
		const mat4f viewProjection(1.0);  // unused here
		quadRenderer.render(viewProjection);
		
		glfwPollEvents();
		glfwSwapBuffers(window);
	}
	
	glfwTerminate();
	return 0;
}
