#include "math/Random.h"
#include "scene/Camera.h"
#include "scene/Mesh.h"
#include "geometry/Grid.h"
#include "io/FileSystem.h"
#include "physics/Cloth.h"

#include "common.h"

using namespace pea;


static int32_t windowWidth = 1280;
static int32_t windowHeight = 720;
static const char* windowTitle = "Cloth Simulation";

static const char* TAG = "Cloth example";
static bool wireframe = false;

/*
1. glMapBuffer/glUnmapBuffer
2. cloth simulation
*/

static void onKey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	std::ignore = window;
	std::ignore = scancode;
	std::ignore = mods;
	switch(key)
	{
	case GLFW_KEY_W:
		if(action == GLFW_PRESS)
			wireframe = !wireframe;
		break;
	
	case GLFW_KEY_ESCAPE:
		if(action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		break;
	}
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
	glfwSetKeyCallback(window, onKey);

	// load all OpenGL function pointers
	loadGL();
	GL::enableDebugMessage();
	
	// Global OpenGL states
	glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	
	int32_t row = 100, column = 100;
	float gap = 0.1;
	float textureStep = gap * 10;
//	Cloth cloth(row, column, gap);
	vec3f ballPosition(7, -5, 0);
	float ballRadius = 2;

	// rectangle cloth
	float curtainWidth = 3, curtainHeight = 2;
	uint32_t sizeX = curtainWidth / gap;
	uint32_t sizeY = curtainHeight / gap;
	std::vector<vec3f> vertices = Grid::getVertexData(sizeX, sizeY, gap);
	std::vector<vec2f> texcoords = Grid::getTexcoordData(1, curtainHeight / static_cast<float>(curtainWidth), sizeX, sizeY);
	std::vector<uint32_t> indices = Grid::getIndexData(sizeX, sizeY, Primitive::QUADRILATERALS);
	
	mat4f transform(1.0);
	transform.rotateX(M_PI / 2);  // (a, b, 0) -> (a, 0, b)
	for(vec3f& vertex: vertices)
	{
		vec4f position(vertex.x, vertex.y, vertex.z, 1.0);
		position = transform * position;
		vertex = vec3f(position.x, position.y, position.z);
	}
	
	// shrink a bit
//	vertices[0].x += gap / 2;
//	vertices[sizeX].x -= gap / 2;
	
	Model model;
	model.addVertex(vertices);
	model.addQuadrilateralFaces(indices.data(), indices.size());

	const std::string groupName = "pinnedGroup";
	Group group(GroupType::VERTEX);
	group.indices.reserve(2);
	group.indices.push_back(0);
	group.indices.push_back(sizeX);
	model.addGroup(groupName, std::move(group));
	
	const std::vector<vec3f>& positions = model.getVertexData();
	std::vector<uint32_t> triangulatedIndices = model.getTriangulatedIndex();

	float mass = 0.5;
	Cloth cloth(model, mass);
	cloth.pinGroup(groupName);

//	const size_t positionSize = positions.size();
	std::vector<vec3f> movingPositions = positions;
//	movingPositions.reserve(positionSize);
/*
	std::cout << "positions.size=" << positions.size() << ", texcoords.size=" << texcoords.size() << '\n';
	for(const vec2f& texcoord: texcoords)
		std::cout << texcoord << '\n';
*/
	std::unique_ptr<Mesh> mesh = Mesh::Builder(positions).
			setTexcoord(std::move(texcoords)).
			setIndex(std::move(triangulatedIndices)).
			build();
	
	mesh->prepare(Primitive::TRIANGLES);
	uint32_t vboFlag[Mesh::VBO_COUNT];
	for(int32_t i = 0; i < Mesh::VBO_COUNT; ++i)
		vboFlag[i] = GL_STATIC_DRAW;
	// it will update vertex position frame by frame
	vboFlag[Shader::ATTRIBUTE_VEC_POSITION] = GL_DYNAMIC_DRAW;
	mesh->upload(vboFlag);
	
	Program program(ShaderFactory::VERT_M_VP_TEXCOORD, ShaderFactory::FRAG_TEXTURE_RGB);
	mesh->setProgram(program.getName());
//	program.printActiveVariables();
	
	Texture texture(GL_TEXTURE_2D);
	std::string path = FileSystem::getRelativePath("res/image/fabric.jpg");
	Texture::Parameter parameter(10);
	parameter.setMapMode(GL_MIRRORED_REPEAT);
	texture.load(path);
	texture.setParameter(parameter);
	mesh->setTexture(texture, 0);
	
	const vec3f cameraPosition(0, -3, -3), viewTarget(0, 0, -3), worldUp(0, 0, 1);
	Camera camera(cameraPosition, viewTarget, worldUp);
	camera.setAspectRatio(static_cast<float>(windowWidth) / windowHeight);
	
	mat4f projectionMatrix = camera.getProjectionMatrix();
	
	const int64_t startTime = getCurrentTime();
	
	int64_t lastTime = startTime;
	while(!glfwWindowShouldClose(window))
	{
		int64_t currentTime = getCurrentTime();
		
		const float t = static_cast<double>(currentTime - startTime) * 1E-3;
		const float dt = static_cast<double>(currentTime - lastTime) * 1E-3;
		lastTime = currentTime;
		
		float angle = t - M_PI / 2;  // start from -90
//		camera.setPosition(vec3f(7.0 * std::cos(angle), 7.0 * std::sin(angle), cameraPosition.z));
		mat4f viewMatrix = camera.getViewMatrix();
		mat4f viewProjection = Camera::multiply(viewMatrix, projectionMatrix);
		
		// clear default frame buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, (wireframe ? GL_LINE: GL_FILL));
		mesh->render(viewProjection);
		glEnable(GL_CULL_FACE);

		if(dt > 0)  // first frame dt = 0
			cloth.step(dt);
		cloth.exportVertextexData(movingPositions);
		
		uint32_t vbo_position = mesh->getVertexBufferObject(Shader::ATTRIBUTE_VEC_POSITION);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
		size_t length = movingPositions.size() * sizeof(vec3f);
#if 1
		vec3f* buffer = static_cast<vec3f*>(glMapBufferRange(GL_ARRAY_BUFFER, 0, length, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
//		std::memcpy(buffer, movingPositions.data(), length);
		std::copy(movingPositions.begin(), movingPositions.end(), buffer);
		glUnmapBuffer(GL_ARRAY_BUFFER);
#else
		// update whole buffer data, using glBufferSubData is fine.
//		glBufferData(GL_ARRAY_BUFFER, length, movingPositions.data(), GL_DYNAMIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, length, movingPositions.data(), GL_DYNAMIC_DRAW);
#endif

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	glfwTerminate();
	return 0;
}
