#include <cinttypes>

#include "opengl/QuadRenderer.h"
#include "opengl/Texture.h"
#include "graphics/Image_PNG.h"
#include "util/utility.h"
#include "io/FileSystem.h"

#include "common.h"

using namespace pea;


static const char* TAG = "voronoi";

static constexpr float CELL_WIDTH = 0.12;
static vec2f cellSize;
static bool showTile = true;
static float weight = 0.5F;


static void onWindowResize(GLFWwindow* window, int width, int height)
{
	(void)window;  // suppress unused parameter warning
	glViewport(0, 0, width, height);
	
	cellSize.width = CELL_WIDTH;
	cellSize.height = CELL_WIDTH / width * height;
}

static void onKey(GLFWwindow* window, int key, int scancode, int action, int mods)
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
	
	case GLFW_KEY_H:
		if(action == GLFW_RELEASE)
			showTile = !showTile;
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

	// Cellular Noise  https://thebookofshaders.com/12/
	std::string fragmentShaderSource = ShaderFactory::VERSION + R""(
layout(location = 11) uniform vec2 size;
layout(location = 15) uniform float time;

in vec2 _texcoord;

out vec4 fragColor;

// https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
// use https://github.com/ashima/webgl-noise in OpenGL ES
vec2 random(vec2 point)
{
	vec2 v = vec2(dot(point, vec2(127.1, 311.7)), dot(point, vec2(269.5, 183.3)));
	return fract(sin(v) * 43758.5453);
}

void main()
{
	vec2 coordinate = _texcoord / size;
	vec2 cell = floor(coordinate);
	vec2 fractional = fract(coordinate);
	
	float min_distance = 1.0;  // minimum distance initializes to max value
	for(int i = 0; i < 9; ++i)
	{
		vec2 offset = vec2(i % 3 - 1, i / 3 -1);  // {-1, 0, 1}
		vec2 point = random(cell + offset);
		point = sin(3.1415926 * point + time);  // variation with time
		point = 0.5 * point + 0.5;  // map [-1.0, +1.0] to [0.0, 1.0].
		point += offset;
		float distance = length(point - fractional);
		if(distance < min_distance)
			min_distance = distance;
	}
#if 0
	vec3 color = vec3(min_distance);
	
	// draw cell center with white color
//	color += 1 - step(0.01, min_distance);
	
	// draw grid line with red color
//	vec2 nearLine = step(vec2(0.98, 0.98), fractional);
//	color.r += nearLine.x + nearLine.y;
	
	fragColor = vec4(color, 1.0);
#else
//	if(min_distance > 0.4)
//		min_distance = 0.0;
	fragColor = vec4(min_distance);
#endif
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
This program demonstrates water caustic effect.
strike H to show/hide tiles under water.
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
	int32_t windowHeight = 640;
	const char* windowTitle = "Voronoi";
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, nullptr/* monitor */, nullptr/* share */);
	if(!window)
	{
		slog.e(TAG, "Failed to open GLFW window. Not OpenGL 3.3 compatible");
		glfwTerminate();
		return -2;
	}
	
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, ::onWindowResize);
	glfwSetKeyCallback(window, onKey);

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
	texture0.load(FileSystem::getRelativePath("res/image/tiles.jpg"), parameter);

	quadRenderer.setProgram(program.getName());
	slog.d(TAG, "program %s", program.getActiveVariables().c_str());
	
	cellSize.width = CELL_WIDTH;
	cellSize.height = CELL_WIDTH / windowWidth * windowHeight;

	Program quadProgram(ShaderFactory::VERT_TEXCOORD, ShaderFactory::FRAG_TEXTURE_RGBA);
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	int64_t startTimeUs = getCurrentTime();
	const mat4f viewProjection(1.0);  // unused here
	while(!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT);
		
		if(showTile)
		{
			quadRenderer.setProgram(quadProgram.getName());
			quadRenderer.setTexture(&texture0);
			quadRenderer.render(viewProjection);
			
			glEnable(GL_BLEND);
		}
		
		int64_t currentTimeUs = getCurrentTime();
		float time = static_cast<float>(currentTimeUs - startTimeUs) * 1E-3;
		program.use();
		quadRenderer.setTexture(nullptr);
		Program::setUniform(Shader::UNIFORM_VEC_SIZE, cellSize);
		Program::setUniform(Shader::UNIFORM_FLT_TIME, time);
		quadRenderer.setProgram(program.getName());
		quadRenderer.render(viewProjection);
		
		if(showTile)
		{
			glDisable(GL_BLEND);
		}
		glfwPollEvents();
		glfwSwapBuffers(window);
	}
	
	glfwTerminate();
	return 0;
}
