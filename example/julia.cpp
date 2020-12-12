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
	bool  dirty;
public:
	JuliaParameters();
	
	std::string toString() const;
};

JuliaParameters::JuliaParameters():
		constant(-0.662, 0.343),  // value can be tweaked!
		center(0, 0),
		scale(1.0),
		dirty(true)
{
}

std::string JuliaParameters::toString() const
{
	std::ostringstream oss;
	oss << "constant" << '=' << '(' << constant.x << ", " << constant.y << ')' << ", "
			<< "offset" << '=' << '(' << center.x << ", " << center.y << ')' << ", "
			<< "scale" << '=' << scale;
	return oss.str();
}

inline void setJuliaParameters(GLFWwindow* window, JuliaParameters* parameters)
{
	glfwSetWindowUserPointer(window, parameters);
}

inline JuliaParameters* getJuliaParameters(GLFWwindow* window)
{
	return static_cast<JuliaParameters*>(glfwGetWindowUserPointer(window));
}

static void onKey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	std::ignore = scancode;
	std::ignore = mods;
	if(action != GLFW_RELEASE)
		return;
	
	JuliaParameters* parameters = getJuliaParameters(window);
	assert(parameters);
	parameters->dirty = true;
	
	bool isControlDown = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
			glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
	bool isShiftDown = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
			glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
	float step = isControlDown? 0.1F: (isShiftDown? 0.01F: 0.001F);
	
	switch(key)
	{
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		break;
	
	case GLFW_KEY_A: parameters->center.x -= step; break;
	case GLFW_KEY_D: parameters->center.x += step; break;
	case GLFW_KEY_S: parameters->center.y -= step; break;
	case GLFW_KEY_W: parameters->center.y += step; break;
	
	case GLFW_KEY_LEFT:  parameters->constant.x -= step; break;
	case GLFW_KEY_RIGHT: parameters->constant.x += step; break;
	case GLFW_KEY_DOWN:  parameters->constant.y -= step; break;
	case GLFW_KEY_UP:    parameters->constant.y += step; break;

	case GLFW_KEY_P:
	{
		std::string text = parameters->toString();
		if(isShiftDown)
			slog.d(TAG, "%s", text.c_str());
		else
		{
			vec2i size = getWindowSize(window);
			snapshot(text + ".png", size.width, size.height);
		}
	}
		break;
	
	default:
		// Only dirty when any keys above is striked.
		parameters->dirty = false;
		break;
	}
}

vec2f calculatePosition(GLFWwindow* window, double x, double y)
{
	double x0, y0;
	glfwGetCursorPos(window, &x0, &y0);
	int32_t width, height;
	glfwGetWindowSize(window, &width, &height);
	
	vec2f position;
	position.x = static_cast<float>(x0) / width;
	position.y = 1.0F - static_cast<float>(y0) / height;
	position = 2.0F * position - vec2f(1.0F, 1.0F);  // map [0, 1] to [-1, +1]
	return position;
}

static void onMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
	std::ignore = xoffset;

	JuliaParameters* parameters = getJuliaParameters(window);
	assert(parameters);
	
	bool isControlDown = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
			glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
	float factor = isControlDown? std::sqrt(2.0): std::pow(2, 1.0 / 12);
	float scaleFactor = yoffset > 0? factor: 1 / factor;
	parameters->scale *= scaleFactor;

	vec2f position = calculatePosition(window, xoffset, yoffset);
	
	// scale with respect to the current mouse position
	parameters->center = (parameters->center - position) * scaleFactor + position;
	
	parameters->dirty = true;
//	slog.d(TAG, "yoffset=%f, scaleFactor=%f, scale=%f", yoffset, scaleFactor, parameters->scale);
}

static vec2f downPosition;
static vec2f downCenter;
static void onMouse(GLFWwindow* window, int button, int action, int mods)
{
	if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		double downX, downY;
		glfwGetCursorPos(window, &downX, &downY);
		downPosition = calculatePosition(window, downX, downY);
		
		downCenter = getJuliaParameters(window)->center;
	}
}

static void onCursorMove(GLFWwindow* window, double x, double y)
{
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if(state == GLFW_PRESS)
	{
		vec2f movedPosition = calculatePosition(window, x, y);
		JuliaParameters* parameters = getJuliaParameters(window);
		parameters->center = downCenter - (movedPosition - downPosition) * parameters->scale;
		parameters->dirty = true;
	}
}

static void onWindowResize(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	
	JuliaParameters* parameters = getJuliaParameters(window);
	assert(parameters);
	parameters->aspectRatio = static_cast<float>(width) / height;
	parameters->dirty = true;
}

int main()
{
	const char* usage = R""(Julia fractal: z = z^2 + C

Usage:
- Use direction keys to tweak constant C, left/right for X value, down/up for Y value.
- Use ADSW keys to move window of the complex plane.
- Combined with Shift key, you get big step.
- Combined with Control key, you get bigger step. When Control and Shift are down, Control prevails.
- Use MMB key to zoom in/out the window to a fine-grain extend.
- move mouse with LMB down to window of the complex plane.
- Use P key to snapshot current frame, Shift + P prints current parameters.
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
	glfwSetMouseButtonCallback(window, onMouse);
	glfwSetCursorPosCallback(window, onCursorMove);
	glfwSetScrollCallback(window, onMouseScroll);

	loadGL();
	GL::enableDebugMessage();
	glDisable(GL_DEPTH_TEST);
	
	JuliaParameters parameters;
	parameters.aspectRatio = static_cast<float>(windowWidth) / windowHeight;
	setJuliaParameters(window, &parameters);
/*
	        point: xy
	1--------0  0: ++
	|        |  1: -+
	|        |  2: --
	2--------3  3: +-
*/
	std::string vertexShaderSource = ShaderFactory::VERSION + R""(
uniform vec2 center;
uniform float scale;
uniform float aspectRatio;

noperspective out vec2 z0;

void main()
{
	vec2 vertex;
	if(aspectRatio >= 1.0)
		vertex = vec2(aspectRatio, 1.0);
	else
		vertex = vec2(1.0, 1.0 / aspectRatio);
	
#if 0
	const vec2 vertices[4] = vec2[4](
			vec2(+vertex.x, +vertex.y),
			vec2(-vertex.x, +vertex.y),
			vec2(-vertex.x, -vertex.y),
			vec2(+vertex.x, -vertex.y));
	vec2 vertex = vertices[gl_VertexID];
#else
	float x = 1.5 - float(gl_VertexID);
	vertex *= sign(vec2(abs(x) - 1.0, x));
#endif
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
		color = vec4(texture(colorMap, x).rgb, 1.0);
	}
}
)"";

	Program program(vertexShaderSource, fragmentShaderSource);
	slog.i(TAG, "%s", program.getActiveVariables().c_str());
	int32_t centerLocation      = program.getUniformLocation("center");
	int32_t scaleLocation       = program.getUniformLocation("scale");
	int32_t constantLocation    = program.getUniformLocation("constant");
	int32_t aspectRatioLocation = program.getUniformLocation("aspectRatio");
	
	Texture texture = createThermosTexture(256);
	
	// https://www.knronos.org/opengl/wiki/Vertex_Rendering
	// A non-zero Vertex Array Object must be bound (though no arrays have to be enabled, so it can be a freshly-created vertex array object).
	uint32_t vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	while(!glfwWindowShouldClose(window))
	{
		if(parameters.dirty)
		{
			program.use();
			Program::setUniform(centerLocation, parameters.center);
			Program::setUniform(scaleLocation, parameters.scale);
			Program::setUniform(constantLocation, parameters.constant);
			Program::setUniform(aspectRatioLocation, parameters.aspectRatio);
			
			glClear(GL_COLOR_BUFFER_BIT);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			glfwSwapBuffers(window);
			
			parameters.dirty = false;
		}
		
		glfwWaitEvents();
	}
	
	glDeleteVertexArrays(1, &vao);
	
	glfwTerminate();
	return 0;
}
