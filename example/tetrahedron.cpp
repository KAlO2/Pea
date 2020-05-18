#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <chrono>
#include <memory>
#include <queue>

#include "geometry/Tetrahedron.h"
#include "opengl/Texture.h"
#include "graphics/Image_PNG.h"
#include "util/utility.h"
#include "io/FileSystem.h"

#include "common.h"

using namespace pea;


static int32_t windowWidth = 640;
static int32_t windowHeight = 640;
static const char* windowTitle = "Tetrahedron";

static const char* TAG = "Tetrahedron example";

/* 
 * Instancing, or instanced rendering, is a way of executing the same drawing commands many times 
 * in a row, with each producing a slightly different result. This can be a very efficient method 
 * of rendering a large amount of geometry with very few API calls.
 */
#define USE_INSTANCING 1

static inline int32_t round(float x)
{
	return static_cast<int32_t>(x);// + 0.5F);
}

/*
	In this lesson, you will learn how to create 
	http://en.wikipedia.org/wiki/Sierpinski_triangle
*/

void fillBottomFlatTriangle(Image& image, const vec2f& point1, const vec2f& point2, const vec2f& point3, uint32_t color)
{
	assert(round(point2.y) == round(point3.y));
	float invSlope1 = static_cast<float>(point2.x - point1.x) / (point2.y - point1.y);
	float invSlope2 = static_cast<float>(point3.x - point1.x) / (point3.y - point1.y);
	float x1 = point1.x, x2 = point1.x;
	for(int32_t y = point1.y; y <= point2.y; ++y)
	{
		int32_t _x1 = std::max(round(x1), 0);
		int32_t _x2 = std::min(round(x2), image.getWidth() - 1);
		for(int32_t x = _x1; x < _x2; ++x)
			image.setPixel(x, y, color);
		x1 += invSlope1;
		x2 += invSlope2;
	}
}

void fillTopFlatTriangle(Image& image, const vec2f& point1, const vec2f& point2, const vec2f& point3, uint32_t color)
{
	assert(round(point1.y) == round(point2.y));
	float invSlope0 = static_cast<float>(point3.x - point1.x) / (point3.y - point1.y);
	float invSlope1 = static_cast<float>(point3.x - point2.x) / (point3.y - point2.y);
	float x0 = point3.x, x1 = point3.x;
	for(int32_t y = point3.y; y > point1.y; --y)
	{
		int32_t _x0 = std::max(round(x0), 0);
		int32_t _x1 = std::min(round(x1), image.getWidth() - 1);
		for(int32_t x = _x0; x < _x1; ++x)
			image.setPixel(x, y, color);
		x0 -= invSlope0;
		x1 -= invSlope1;
	}
}

// http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html
void fillTriangle(Image& image, const vec2f& point1, const vec2f& point2, const vec2f& point3, uint32_t color)
{
	int32_t p1y = round(point1.y), p2y = round(point2.y), p3y = round(point3.y);
	assert(p1y <= p2y && p2y <= p3y);
	if(p2y == p3y)
		fillBottomFlatTriangle(image, point1, point2, point3, color);
	else if(p1y == p2y)
		fillTopFlatTriangle(image, point1, point2, point3, color);
	else
	{
		float k = static_cast<float>(point3.y - point1.y) / (point3.x - point1.x);
		vec2f point4(point1.x + (point2.y - point1.y) / k, point2.y);
		fillBottomFlatTriangle(image, point1, point2, point4, color);
		fillTopFlatTriangle(image, point2, point4, point3, color);
	}
}

void fillTriangleRecursive(Image& image, const vec2f& point1, const vec2f& point2, const vec2f& point3)
{
	auto lessThan = [](const vec2f& lhs, const vec2f& rhs)
	{
		return round(lhs.y) < round(rhs.y) || (round(lhs.y) == round(rhs.y) && lhs.x <= rhs.y);
	};
	vec2f p[3] = {point1, point2, point3};
	std::sort(p, p + 3, lessThan);
	
	vec2f p01 = (p[0] + p[1]) / 2;
	vec2f p12 = (p[1] + p[2]) / 2;
	vec2f p02 = (p[2] + p[0]) / 2;
	
	if(std::abs(cross(p[1] - p[0], p[2] - p[0])) < 32)
		return;

	constexpr uint32_t WHITE = 0xFFFFFFFF, RED = 0xFF0000FF, GREEN = 0xFF00FF00, BLUE = 0xFFFF0000;
	fillTriangle(image, p[0], p01, p02, RED);
	fillTriangle(image, p01, p[1], p12, GREEN);
	fillTriangle(image, p02, p12, p[2], BLUE);
	fillTriangle(image, p01, p12, p02, WHITE);
	
	fillTriangleRecursive(image, p[0], p01, p02);
	fillTriangleRecursive(image, p01, p[1], p12);
	fillTriangleRecursive(image, p02, p12, p[2]);
//	fillTriangleRecursive(image, p01, p02, p12);
}

void sierpinski2(Image& image)
{
	assert(image.getWidth() == image.getHeight());
	uint32_t size = image.getWidth();
	vec2f point1(0, 0), point2(size, 0);
	vec2f point3(size * 0.5, size * (sqrt(3.0) / 2));
	
	fillTriangleRecursive(image, point1, point2, point3);
}

/**
 * @param[in] weight Edge length of a regular tetrahedron.
 * @param[in] depth Subdivision level.
 * @return 4^depth vertex data, each vec4 vertex consist of vec3 center position and float weight.
 */
std::vector<vec4f> createInstances(float weight, int32_t depth)
{
	assert(weight > 0 && depth >= 0);

	vec4f root(0, 0, 0, weight);
	if(depth == 0)
		return std::vector<vec4f>({root});
	
	std::vector<vec3f> positions = Tetrahedron::getVertexData(weight);
	assert(positions.size() == 4);
	
#if 1
	std::queue<vec4f> queue;
	queue.push(root);
	const float lowerWeight = weight / depth;
	while(true)
	{
		const vec4f& vertex = queue.front();
		const float& w = vertex.w;
		if(w < lowerWeight)
			break;
		
		vec3f v[4];
		for(int8_t i = 0; i < 4; ++i)
			v[i] = vec3f(vertex.x, vertex.y, vertex.z) + positions[i] * w;
		
		vec3f v01 = (v[0] + v[1]) / 2;
		vec3f v02 = (v[0] + v[2]) / 2;
		vec3f v03 = (v[0] + v[3]) / 2;
		vec3f v12 = (v[1] + v[2]) / 2;
		vec3f v13 = (v[1] + v[3]) / 2;
		vec3f v23 = (v[2] + v[3]) / 2;
		
		vec3f n0 = (v[0] + v01 + v02 + v03) / 4;
		vec3f n1 = (v01 + v[1] + v12 + v13) / 4;
		vec3f n2 = (v02 + v12 + v[2] + v23) / 4;
		vec3f n3 = (v03 + v13 + v23 + v[3]) / 4;
		float nw = w / 2;
		
		queue.push(vec4f(n0.x, n0.y, n0.z, nw));
		queue.push(vec4f(n1.x, n1.y, n1.z, nw));
		queue.push(vec4f(n2.x, n2.y, n2.z, nw));
		queue.push(vec4f(n3.x, n3.y, n3.z, nw));
		queue.pop();
	}

	// turn std::queue into std::vector
	std::vector<vec4f> vertices;
	vertices.reserve(queue.size());
	while(!queue.empty())
	{
		vertices.push_back(queue.front());
		queue.pop();
	}
#else
	// non recursive method
	int32_t count = pea::pow<int32_t>(4, depth);
	std::vector<vec4f> vertices(size);
	vertices[0] = root;
	for(int32_t d = 1; d < depth; ++d)
	{
		int32_t count = pea::pow<int32_t>(4, d);
		for(int32_t i = count - 1; i >= 0; --i)
		{
			// TODO
		}
	}
#endif
	return vertices;
}

void onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	std::ignore = scancode;
	std::ignore = mods;
	
	switch(key)
	{
	case GLFW_KEY_ESCAPE:
		if(action == GLFW_RELEASE)
			glfwSetWindowShouldClose(window, true);
		break;
	
	case GLFW_KEY_LEFT:
	case GLFW_KEY_RIGHT:
/*
		if(action == GLFW_PRESS)
		{
			bool isControlPressed = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS
					|| glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
			float step = isControlPressed? 0.05F: 0.005F;
			if(key == GLFW_KEY_LEFT)
				weight -= step;
			else
				weight += step;
			weight = clamp(weight, 0.0F, 1.0F);
			slog.d(TAG, "control=%d, left weight=%.3f", static_cast<int>(isControlPressed), weight);
		}
*/
		break;
	}
}

uint32_t createProgram()
{
#if USE_INSTANCING
	constexpr char _ = ' ';
	std::ostringstream oss;
	oss << "#define" << _ << "USE_INSTANCING" << _ << USE_INSTANCING << '\n';
	std::string macros = oss.str();
	
	std::string vertexShaderSource = ShaderFactory::VERSION + macros + R""(
#if USE_INSTANCING
layout(location = 6) in vec4 instance;
#else
layout(location = 0) uniform mat4 model;
#endif

layout(location = 4) uniform mat4 view;
layout(location = 8) uniform mat4 projection;

layout(location = 0) in vec3 position;
layout(location = 3) in vec2 texcoord;

out vec2 _texcoord;

void main()
{
#if USE_INSTANCING
	mat4 model = mat4(
			instance.w, 0, 0, 0,
			0, instance.w, 0, 0,
			0, 0, instance.w, 0,
			instance.x, instance.y, instance.z, 1);
#endif
	gl_Position = projection * (view * (model * vec4(position, 1.0)));
	_texcoord = texcoord;
}
)"";

	uint32_t vertexShader = ShaderFactory::loadShader(Shader::VERTEX_SHADER, vertexShaderSource);
#else
	uint32_t vertexShader = ShaderFactory::loadShader(ShaderFactory::VERT_TRANSFORM_TEXTURE);
#endif
	uint32_t fragmentShader = ShaderFactory::loadShader(ShaderFactory::FRAG_TEXTURE_RGBA);
	assert(vertexShader != 0 && fragmentShader != 0);
	uint32_t program = Program::createProgram(2, vertexShader, fragmentShader);
	slog.i(TAG, "create program %" PRIu32, program);
	
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	
	return program;
}

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
	uint32_t program = createProgram();
	assert(program != 0);
	
	// per vertex data
	constexpr float edgeLength = 2.0F;
	std::vector<vec3f> positions = Tetrahedron::getVertexData(edgeLength);
	std::vector<uint8_t> indices = Tetrahedron::getVertexIndex(Primitive::TRIANGLES);
	std::vector<vec2f> texcoords = Tetrahedron::getTexcoordData();
	size_t vertexCount = Tetrahedron::getIndexSize();
	std::vector<vec3f> vertices(vertexCount);
	for(size_t i = 0; i < vertexCount; ++i)
		vertices[i] = positions[indices[i]];
	positions = vertices;

	assert(positions.size() == vertexCount);
	assert(texcoords.size() == vertexCount);
	
	// per instance data
	const std::vector<vec4f> instances = createInstances(edgeLength, 3);
	slog.d(TAG, "instances.size() = %zu", instances.size());
#if USE_INSTANCING
	uint32_t vbo[3];
	const auto& [vbo_position, vbo_texcoord, vbo_instance] = vbo;
#else
	uint32_t vbo[2];
	const auto& [vbo_position, vbo_texcoord] = vbo;
#endif
	glGenBuffers(sizeofArray(vbo), vbo);
	
	uint32_t vao[1];
	glGenVertexArrays(sizeofArray(vao), vao);
	glBindVertexArray(vao[0]);
	GL::bindVertexBuffer(vbo_position, Shader::ATTRIBUTE_VEC_POSITION, positions);
	GL::bindVertexBuffer(vbo_texcoord, Shader::ATTRIBUTE_VEC_TEXCOORD, texcoords);
#if USE_INSTANCING
	GL::bindVertexBuffer(vbo_instance, Shader::ATTRIBUTE_VEC_INSTANCE, instances);
	glVertexAttribDivisor(Shader::ATTRIBUTE_VEC_INSTANCE, 1);
#endif
	glBindVertexArray(0);
	
	Image_PNG image(1024, 1024, Color::Format::RGBA_8888);
	image.fillColor(0xFFFFFFFF);  // transparent
	
	sierpinski2(image);
//	image.flipVertical();
//	image.fillCheckerboard(256);
//	image.save("sierpinski.png");

	Texture texture0;
	int32_t maxLevel = 10;  //(std::max(image.getWidth(), image.getHeight()));
	Texture::Parameter parameter(maxLevel);
	glActiveTexture(GL_TEXTURE0);
	texture0.load(image, parameter);

#if USE_INSTANCING
	assert(glGetAttribLocation(program, "instance") == Shader::ATTRIBUTE_VEC_INSTANCE);
#else
	assert(glGetUniformLocation(program, "model") == Shader::UNIFORM_MAT_MODEL);
#endif
	assert(glGetUniformLocation(program, "view") == Shader::UNIFORM_MAT_VIEW);
	assert(glGetUniformLocation(program, "projection") == Shader::UNIFORM_MAT_PROJECTION);
	assert(glGetAttribLocation(program, "position") == Shader::ATTRIBUTE_VEC_POSITION);

//	mat4f modelMatrix(1.0f);
	
	float orbit = 4;
	vec3f position(0, -orbit, 0), target(0, 0, 0), up(0, 0, 1);
	mat4f viewMatrix = Camera::lookAt(position, target, up);
	
	float fieldOfView = M_PI / 3;
	float aspectRatio = static_cast<float>(windowWidth) / windowHeight;
	float nearValue = 1.0F, farValue = 1024.0F;
	mat4f projectionMatrix = Camera::perspective(fieldOfView, aspectRatio, nearValue, farValue);
	
	glUseProgram(program);
	Program::setUniform(Shader::UNIFORM_MAT_PROJECTION, projectionMatrix);
	
//	glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
	glEnable(GL_DEPTH_TEST);

//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	auto start = std::chrono::system_clock::now();
int32_t frameIndex = 0;
	while(!glfwWindowShouldClose(window))
	{
		// clear default frame buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// step 1: use program
		// step 2: set attributes and uniforms
		// step 3: draw
//		Program::setUniform(textureLocation, i);
		// int64_t microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> diff = end - start;
		float t = diff.count() / 2;  // in second
		position = vec3f(orbit * std::cos(t), orbit * std::sin(t), 0);
		mat4f viewMatrix = Camera::lookAt(position, target, up);
		Program::setUniform(Shader::UNIFORM_MAT_VIEW, viewMatrix);
		
		glBindVertexArray(vao[0]);
#if USE_INSTANCING
		glDrawArraysInstanced(GL_TRIANGLES, 0, vertexCount, instances.size());
#else
		for(const vec4f& instance: instances)
		{
			// instance consists of scaling (w) and translation (x, y, z).
			// [w, 0, 0, x]
			// [0, w, 0, y]
			// [0, 0, w, z]
			// [0, 0, 0, 1]
			mat4f modelMatrix = mat4f(1.0f)
					.scale(instance.w)
					.translate(vec3f(instance.x, instance.y, instance.z));
					//.rotateX(t);  // .rotate(vec3f(1, 0, 0), t); ???

			Program::setUniform(Shader::UNIFORM_MAT_MODEL, modelMatrix);
			glDrawArrays(GL_TRIANGLES, 0, vertexCount);
			
		}
#endif
/*
		std::ostringstream oss;
		oss << "tetrahedron." << std::setw(3) << std::setfill('0') << frameIndex << ".png";
		snapshot(oss.str(), windowWidth, windowHeight);
		slog.d(TAG, "save image %s", oss.str().c_str());
		++frameIndex;
*/
		glfwSwapBuffers(window);
		
		// poll IO events (keys pressed/released, mouse moved etc.)
		glfwPollEvents();
	}

	// de-allocate all resources once they've outlived their purpose
	glDeleteVertexArrays(sizeofArray(vao), vao);
	glDeleteBuffers(sizeofArray(vbo), vbo);
	glDeleteProgram(program);
	slog.i(TAG, "delete program %" PRIu32, program);

	glfwTerminate();
	return 0;
}
