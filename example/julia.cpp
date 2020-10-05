#include "common.h"

using namespace pea;

static const char* TAG = "Julia";
/*
In this example, you will learn two person: Julia and Mandelbrote.
First and foremost, the fractal world is facinating!
Second, No vertex attribute data is uploaded to GPU, gl_VertexID will do the trick under the hood.
*/
class JuliaParameters
{
public:
	vec2f constant;
	vec2f center;
	float scale;
	float aspectRatio;
	bool animate;
	bool invalidate;
public:
	JuliaParameters();
	
	std::string toString() const;
};

JuliaParameters::JuliaParameters():
		constant(-0.662, 0.343),  // value can be tweaked!
		center(0, 0),
		scale(1.0),
		animate(false),
		invalidate(true)
{
}

std::string JuliaParameters::toString() const
{
	std::ostringstream oss;
	oss << "c=(" << constant.x << ", " << constant.y << "), "
			<< "offset=(" << center.x << ", " << center.y << "), "
			<< "scale=" << scale;
	return oss.str();
}

static void onKey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	std::ignore = window;
	std::ignore = scancode;
	std::ignore = mods;
	
	JuliaParameters* parameters = static_cast<JuliaParameters*>(glfwGetWindowUserPointer(window));
	assert(parameters);
	
	bool isControlDown = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
			glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
	bool isShiftDown = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
			glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
	float step;
	if(isControlDown)
		step = 0.1;
	else if(isShiftDown)
		step = 0.01;
	else
		step = 0.001;
	
	switch(key)
	{
	case GLFW_KEY_ESCAPE:
		if(action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		break;
	
	case GLFW_KEY_A:
		parameters->center.x -= step;
		break;
	case GLFW_KEY_D:
		parameters->center.x += step;
		break;
	case GLFW_KEY_S:
		parameters->center.y -= step;
		break;
	case GLFW_KEY_W:
		parameters->center.y += step;
		break;
		
	case GLFW_KEY_LEFT:
		parameters->constant.x -= step;
		break;
	case GLFW_KEY_RIGHT:
		parameters->constant.x += step;
		break;
	case GLFW_KEY_DOWN:
		parameters->constant.y -= step;
		break;
	case GLFW_KEY_UP:
		parameters->constant.y += step;
		break;

	case GLFW_KEY_SPACE:
		if(action == GLFW_RELEASE)
			parameters->animate = !parameters->animate;
		break;
	}
	
	parameters->invalidate = true;
//	slog.d(TAG, "%s", parameters->toString().c_str());
}

void onMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
	std::ignore = window;
	std::ignore = xoffset;

	JuliaParameters* parameters = static_cast<JuliaParameters*>(glfwGetWindowUserPointer(window));
	assert(parameters);
	
	bool isControlDown = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
			glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
	float factor = isControlDown? std::sqrt(2.0): std::pow(2, 1.0 / 12);
	float scaleFactor = yoffset > 0? factor: 1 / factor;
	parameters->scale *= scaleFactor;
#if 1
	vec2i mousePosition = getMousePosition(window);
	vec2i windowSize = getWindowSize(window);
	vec2f position;  // [-aspectRatio, +aspectRatio] x [-1, +1]
	position.x = static_cast<float>(mousePosition.x) / windowSize.width;
	position.y = 1.0 - static_cast<float>(mousePosition.y) / windowSize.height;
	position = 2.0F * position - vec2f(1.0F, 1.0F);  // map [0, 1] to [-1, +1]
	position.x *= parameters->aspectRatio;  // with respect to aspect ratio
	
	// scale with respect to mouse position
	parameters->center = (parameters->center - position) * scaleFactor + position;
#endif
	
	parameters->invalidate = true;
//	slog.d(TAG, "yoffset=%f, scaleFactor=%f, scale=%f", yoffset, scaleFactor, parameters->scale);
}

static void onWindowResize(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	
	JuliaParameters* parameters = static_cast<JuliaParameters*>(glfwGetWindowUserPointer(window));
	assert(parameters);
	parameters->aspectRatio = static_cast<float>(width) / height;
	parameters->invalidate = true;
}

int main()
{
	const char* usage = R""(
Julia fractal: z = z^2 + constant

Usage:
- use direction keys to tweak constant, left/right for X value, down/up for Y value.
- use ADSW keys to adjust window of the complex plane.
- combined with Ctrl key, you get bigger step.
- MMB key zoom in/out the window to a fine-grain extend.
)"";
	printf("%s\n", usage);
	
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
	const char* windowTitle = "Julia Example";
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
	glfwSetScrollCallback(window, onMouseScroll);

	loadGL();
	GL::enableDebugMessage();
	
	JuliaParameters parameters;
	parameters.aspectRatio = static_cast<float>(windowWidth) / windowHeight;
	glfwSetWindowUserPointer(window, &parameters);
	std::string vertexShaderSource = ShaderFactory::VERSION + R""(
uniform vec2 center;
uniform float scale;
uniform float aspectRatio;

noperspective out vec2 z0;

void main()
{
	const vec2 vertices[4] = vec2[4](
			vec2(+aspectRatio, +1.0),
			vec2(-aspectRatio, +1.0),
			vec2(-aspectRatio, -1.0),
			vec2(+aspectRatio, -1.0)
	);
	
	vec2 vertex = vertices[gl_VertexID];
	z0 = center + vertex * scale;
	gl_Position = vec4(vertex, 0.5, 1.0);
}
)"";
	
	std::string fragmentShaderSource = ShaderFactory::VERSION + R""(
uniform sampler1D colorMap;
uniform vec2 constant;

noperspective in vec2 z0;

out vec4 color;

void main()
{
	const int maxIteration = 256;

	vec2 z = z0;
	int iteration;
	for(iteration = 0; iteration < maxIteration; ++iteration)
	{
		// z = x + y*j;  z^2 = (x^2 - y^2) + 2*x*y*j;
		float x = z.x * z.x - z.y * z.y;
		float y = 2.0 * z.x * z.y;
		z = vec2(x, y) + constant;
		
		const float thresholdSquared = 16.0;  // [-1.5, +1.5] x [-1, +1]
		if(dot(z, z) > thresholdSquared)
			break;
	}
	
	if(iteration >= maxIteration)
		color = vec4(0.0, 0.0, 0.0, 1.0);  // black
	else
	{
		float x = float(iteration) / float(maxIteration);
//		color = vec4(x, (z.x + 1) * 0.5, (z.y + 1) * 0.5, 1);
		color = vec4(texture(colorMap, x).rgb, 1.0);
	}
}
)"";

	uint32_t vertexShader   = ShaderFactory::loadShader(Shader::VERTEX_SHADER, vertexShaderSource);
	uint32_t fragmentShader = ShaderFactory::loadShader(Shader::FRAGMENT_SHADER, fragmentShaderSource);
	Program program(Program::createProgram(2, vertexShader, fragmentShader));
	slog.i(TAG, "%s", program.getActiveVariables().c_str());
	
	int32_t centerLocation   = program.getUniformLocation("center");
	int32_t scaleLocation     = program.getUniformLocation("scale");
	int32_t constantLocation = program.getUniformLocation("constant");
	int32_t aspectRatioLocation = program.getUniformLocation("aspectRatio");
	
	Texture texture = createThermosTexture(256);
	
	// https://www.knronos.org/opengl/wiki/Vertex_Rendering
	// A non-zero Vertex Array Object must be bound (though no arrays have to be enabled, so it can be a freshly-created vertex array object).
	uint32_t vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	while(!glfwWindowShouldClose(window))
	{
		if(parameters.invalidate || parameters.animate)
		{
			// clear default frame buffer
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
			int64_t time = getCurrentTime();
			float t = static_cast<float>(time) * 1E-3;
			vec2f constant = parameters.animate? vec2f(std::cos(2.1f * t + 0.23), std::sin(4.8f * t - 0.07)): parameters.constant;
			
			program.use();
			Program::setUniform(centerLocation, parameters.center);
			Program::setUniform(scaleLocation, parameters.scale);
			Program::setUniform(constantLocation, constant);
			Program::setUniform(aspectRatioLocation, parameters.aspectRatio);
			
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

			glfwSwapBuffers(window);
			
			parameters.invalidate = false;
		}
		glfwPollEvents();
	}
	
	glDeleteVertexArrays(1, &vao);
	
	glfwTerminate();
	return 0;
}
