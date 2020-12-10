#include "math/Random.h"
#include "math/function.h"

#include "scene/Object.h"
#include "util/utility.h"

#include "common.h"

#include <cstdlib>  // std::atoi
#include <cstring>  // std::strcmp

#include <getopt.h>

using namespace pea;

static const char* TAG = "PointCloud";

using IndexType = uint8_t;  // change to uint16_t or uint32_t if not fit.
enum Type: uint32_t
{
	SQUARE = 0,
	CUBE,
	DISK,
	SPHERE,
//	COUNT,
};

class PointCloudViewer: private Object
{
private:
	vec4f pointColor;
	vec4f lineColor;
	
	std::vector<vec3f> points;       // point data
	std::vector<vec3f> vertices;     // line data
	std::vector<IndexType> indices;  // line data
	
	Program program;
	uint32_t vao[2];  // 0 for point, 1 for line
	uint32_t vbo[3];
	
public:
	PointCloudViewer();
	virtual ~PointCloudViewer();
	
	void setPointData(const std::vector<vec3f>& points);
	void setLineData(const std::vector<vec3f>& vertices, const std::vector<IndexType>& indices);
	
	void setPointColor(const vec4f& color);
	void setLineColor(const vec4f& color);
	
	void uploadVertexData();
	
	void render(const mat4f& viewProjection) const override;
};

PointCloudViewer::PointCloudViewer():
		pointColor(1.0, 1.0, 1.0, 1.0),  // white
		lineColor(1.0, 1.0, 1.0, 1.0),   // white
		program(ShaderFactory::VERT_M_VP, ShaderFactory::FRAG_UNIFORM_COLOR)
{
	glGenVertexArrays(sizeofArray(vao), vao);
	glGenBuffers(sizeofArray(vbo), vbo);
	
	setProgram(program.getName());
	slog.d(TAG, "program = %s", program.getActiveVariables().c_str());
}

PointCloudViewer::~PointCloudViewer()
{
	glDeleteVertexArrays(sizeofArray(vao), vao);
	glDeleteBuffers(sizeofArray(vbo), vbo);
}

void PointCloudViewer::setPointData(const std::vector<vec3f>& points)
{
	this->points = points;
}

void PointCloudViewer::setLineData(const std::vector<vec3f>& vertices, const std::vector<IndexType>& indices)
{
	this->vertices = vertices;
	this->indices = indices;
}

void PointCloudViewer::setPointColor(const vec4f& color)
{
	pointColor = color;
}

void PointCloudViewer::setLineColor(const vec4f& color)
{
	lineColor = color;
}

void PointCloudViewer::uploadVertexData()
{
	glBindVertexArray(vao[0]);
	GL::bindVertexBuffer(vbo[0], Shader::ATTRIBUTE_VEC_POSITION, points);
	
	glBindVertexArray(vao[1]);
	GL::bindVertexBuffer(vbo[1], Shader::ATTRIBUTE_VEC_POSITION, vertices);
	GL::bindIndexBuffer(vbo[2], indices);
	
	glBindVertexArray(0);
}

void PointCloudViewer::render(const mat4f& viewProjection) const
{
	Object::render(viewProjection);
	
	if(!points.empty())
	{
		Program::setUniform(Shader::UNIFORM_VEC_UNIFORM_COLOR, pointColor);
		glBindVertexArray(vao[0]);
		glDrawArrays(GL_POINTS, 0, points.size());
	}
	
	if(!indices.empty())
	{
		GLenum indexType;
		using Type = decltype(indices)::value_type;
		if constexpr(std::is_same<Type, uint8_t>::value)
			indexType = GL_UNSIGNED_BYTE;
		else if constexpr(std::is_same<Type, uint16_t>::value)
			indexType = GL_UNSIGNED_SHORT;
		else if constexpr(std::is_same<Type, uint32_t>::value)
			indexType = GL_UNSIGNED_INT;
		else
			assert(false);  // invalid type
		
		Program::setUniform(Shader::UNIFORM_VEC_UNIFORM_COLOR, lineColor);
		glBindVertexArray(vao[1]);
		glDrawElements(GL_LINES, indices.size(), indexType, nullptr);
	}
}

std::pair<std::vector<vec3f>, std::vector<IndexType>> createSquareLine()
{
	std::vector<vec3f> vertices(4);
	for(uint32_t i = 0; i < 4; ++i)
		vertices[i] = vec3f(i % 2, (i % 4) < 2, 0);
	std::vector<uint8_t> indices
	{
		0, 1, 2, 3,
		0, 2, 1, 3,
	};
	
	return std::make_pair(vertices, indices);
}
/*
	    6---------7
	   /:        /|
	  / :       / |
	 /  :      /  |
	 4--:-----5   |
	 |  2- - -|- -3
	 | /      |  /
	 |/       | /
	 0--------1/
*/
std::pair<std::vector<vec3f>, std::vector<IndexType>> createCubeLine()
{
	std::vector<vec3f> vertices(8);
	for(uint32_t i = 0; i < 8; ++i)
		vertices[i] = vec3f(i % 2, (i % 4) < 2, i < 4);
	
	std::vector<uint8_t> indices
	{
		0, 1, 2, 3, 4, 5, 6, 7,  // X direction
		0, 2, 1, 3, 4, 6, 5, 7,  // Y direction
		0, 4, 1, 5, 2, 6, 3, 7,  // Z direction
	};
	
	return std::make_pair(vertices, indices);
}

std::pair<std::vector<vec3f>, std::vector<IndexType>> createDiskLine()
{
	constexpr uint32_t N = 32;
	std::vector<vec3f> vertices(N);
	std::vector<IndexType> indices(N * 2);
	
	float cost[N + 1], sint[N + 1];
	generateCosineSineTable(cost, sint, N);
	
	for(uint32_t i = 0; i < N; ++i)
	{
		vertices[i] = vec3f(cost[i], sint[i], 0);
		
		indices[i * 2] = i;
		indices[i * 2 + 1] = (i + 1) % N;
	}
	
	return std::make_pair(vertices, indices);
}

std::pair<std::vector<vec3f>, std::vector<IndexType>> createSphereLine()
{
	constexpr uint32_t N = 32;
	static_assert(N * 3 < std::numeric_limits<IndexType>::max());
	
	std::vector<vec3f> vertices;
	std::vector<IndexType> indices;
	
	vertices.reserve(N * 3);
	indices.reserve(N * 6);
	
	float cost[N + 1], sint[N + 1];
	generateCosineSineTable(cost, sint, N);
	
	auto appendIndex = [&indices](uint32_t base)
	{
		for(uint32_t i = 0; i < N; ++i)
		{
			indices.push_back(base + i);
			indices.push_back(base + (i + 1) % N);
		}
	};
	
	for(uint32_t i = 0; i < N; ++i)
		vertices.emplace_back(0, cost[i], sint[i]);
	appendIndex(0);
	
	for(uint32_t i = 0; i < N; ++i)
		vertices.emplace_back(sint[i], 0, cost[i]);
	appendIndex(N);
	
	for(uint32_t i = 0; i < N; ++i)
		vertices.emplace_back(cost[i], sint[i], 0);
	appendIndex(N * 2);
	
	return std::make_pair(vertices, indices);
}

void usage()
{
	const char* manual = R""(DESCRIPTION
  -h, --help
    Show a summary of the options and exit.
  
  -q, --quantity=NUMBER
    number of points to show.
  
  -t, --type=TYPE
    type can be square, cube, disk, or sphere.
)"";

	printf("%s", manual);
}

int main(int argc, char* argv[])
{
	Type type = Type::SQUARE;
	uint32_t quantity = 1000;
	const struct option longOptions[] =
	{
		{"help",           no_argument, nullptr, 'h'},
		{"quantity", required_argument, nullptr, 'n'},
		{"type",     required_argument, nullptr, 't'},
		{nullptr,                    0, nullptr,   0},
	};
	int ch;
	int optionIndex = 0;
	while((ch = getopt_long(argc, argv, "ht:", longOptions, &optionIndex)) != -1)
	{
		switch(ch)
		{
		case 't':
			if(strcmp(optarg, "square") == 0)
				type = Type::SQUARE;
			else if(strcmp(optarg, "cube") == 0)
				type = Type::CUBE;
			else if(strcmp(optarg, "disk") == 0)
				type = Type::DISK;
			else if(strcmp(optarg, "sphere") == 0)
				type = Type::SPHERE;
			else
			{
				printf("unknown type \'%s\', option can be {square, cube, disk, sphere}", optarg);
				return 0;
			}
			break;
		
		case 'q':
			quantity = std::atoi(optarg);
			
			break;
		case 'h':
			usage();
			return 0;
		
		default:
			return 0;
		}
	}
	
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
	const char* windowTitle = __FILE__;
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, nullptr/* monitor */, nullptr/* share */);
	if(!window)
	{
		slog.e(TAG, "Failed to open GLFW window. Not OpenGL 3.3 compatible");
		glfwTerminate();
		return -2;
	}
	
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, onWindowResize);

	loadGL();
	GL::enableDebugMessage();
	
	// global OpenGL state
	glEnable(GL_DEPTH_TEST);
	glPointSize(1.0);
	
	
	std::vector<vec3f> points(quantity);
	std::pair<std::vector<vec3f>, std::vector<IndexType>> lines;

//	Random::setSeed(2020);
	switch(type)
	{
	case Type::SQUARE:
		for(uint32_t i = 0; i < quantity; ++i)
			points[i] = vec3f(Random::emit(), Random::emit(), 0);
		lines = createSquareLine();
		break;
	case Type::CUBE:
		for(uint32_t i = 0; i < quantity; ++i)
			points[i] = vec3f(Random::emit(), Random::emit(), Random::emit());
		lines = createCubeLine();
		break;
	case Type::DISK:
		for(uint32_t i = 0; i < quantity; ++i)
		{
			vec2f point = Random::diskEmit(1.0);
			points[i] = vec3f(point.x, point.y, 0);
		}
		lines = createDiskLine();
		break;
	case Type::SPHERE:
		for(uint32_t i = 0; i < quantity; ++i)
			points[i] = Random::sphereEmit(1.0);
		lines = createSphereLine();
		break;
	}

	std::vector<vec3f>& vertices = lines.first;
	std::vector<IndexType>& indices = lines.second;
	
//	mat4f modelMatrix(1.0F);
//	mat4f viewMatrix;  // updates viewMatrix frame by frame.

	float fieldOfView = M_PI / 3;
	float aspectRatio = static_cast<float>(windowWidth) / windowHeight;
	float nearValue = 1.0F, farValue = 1024.0F;
	
	mat4f projectionMatrix = Camera::perspective(fieldOfView, aspectRatio, nearValue, farValue);
	
	PointCloudViewer viewer;
	viewer.setPointData(points);
	viewer.setLineData(vertices, indices);
	viewer.setPointColor(vec4f(0.1, 1.0, 0.0, 1.0));
	viewer.setLineColor(vec4f(0.1, 1.0, 1.0, 1.0));
//	viewer.setTransform(modelMatrix);  // method made private since transforming a point is pointless.
	viewer.uploadVertexData();
	
	double startTime = glfwGetTime();
	while(!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// Set frame time
		float currentTime = glfwGetTime();
		float elapsedTime = currentTime - startTime;
		
		constexpr float radius = 2.0F;
		vec3f position(radius * std::cos(elapsedTime), radius * std::sin(elapsedTime), 2);
		vec3f target(0.5, 0.5, 0.5), up(0, 0, 1);
		mat4f viewMatrix = Camera::lookAt(position, target, up);
		
		mat4f viewProjection = Camera::multiply(viewMatrix, projectionMatrix);
		viewer.render(viewProjection);
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	glfwTerminate();
	return 0;
}
