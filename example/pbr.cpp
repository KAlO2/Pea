#include "geometry/Sphere.h"
#include "io/FileSystem.h"
#include "opengl/Buffer.h"
#include "scene/Mesh.h"

#include "common.h"

using namespace pea;

static const char* TAG = "pbr";

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
	
	// mesh
	float radius = 1.0;
	Sphere sphere(vec3f(0, 0, 0), radius);
	std::vector<vec3f> vertices = sphere.getVertexData();
	std::vector<vec3f> normals  = sphere.getNormalData();
	constexpr Primitive primitive = Primitive::TRIANGLE_STRIP;
	std::vector<uint32_t> indices = Sphere::getVertexIndex(primitive);
	std::unique_ptr<Mesh> mesh = Mesh::Builder(std::move(vertices))
			.setNormal(std::move(normals))
			.setIndex(std::move(indices))
			.build();
	
	mesh->prepare(primitive);
	mesh->upload();
	
	std::vector<vec3f> lightPositions{vec3f(1, -1.5, 3), vec3f(-2, -0.5, 2.5), vec3f(+2, 0.0, 2)};
	std::vector<vec3f> lightColors{vec3f(0.9, 0.8, 0.7), vec3f(1.0, 1.0, 0.0), vec3f(1.0, 0.0, 1.0)};
	const vec3f* lightPosition = lightPositions.data();
	const vec3f* lightColor = lightColors.data();
	const int32_t lightCount = lightPositions.size();
	
#define USE_TEXTURE 0
#define USE_DYNAMIC_LIGHTS 0
	Program program;
	{
		size_t macroCount = 3 - USE_DYNAMIC_LIGHTS;
		std::vector<std::string> macros;
		macros.reserve(macroCount << 1);
		macros.push_back("USE_TEXTURE");
		macros.push_back(std::to_string(USE_TEXTURE));
		
		macros.push_back("USE_DYNAMIC_LIGHTS");
		macros.push_back(std::to_string(USE_DYNAMIC_LIGHTS));
		if(!USE_DYNAMIC_LIGHTS)
		{
			macros.push_back("LIGHT_COUNT");
			macros.push_back(std::to_string(lightCount));
		}
		uint32_t vertexShader = ShaderFactory::loadShader(ShaderFactory::VERT_M_VP_NORMAL_TEXCOORD);
		uint32_t fragmentShader = ShaderFactory::loadShader(ShaderFactory::FRAG_PBR, macros.data(), macroCount);
		assert(vertexShader != 0 && fragmentShader != 0);
		program = Program(Program::createProgram(2, vertexShader, fragmentShader));
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}
	slog.d(TAG, "program %s", program.getActiveVariables().c_str());
	mesh->setProgram(program.getName());
	
#if USE_DYNAMIC_LIGHTS
	Buffer bufferLightPosition, bufferLightColor;
	bufferLightPosition.bind();
	bufferLightPosition.setData(lightPosition, sizeof(vec3f) * lightCount, Buffer::Usage::STATIC_DRAW);
	bufferLightColor.bind();
	bufferLightColor.setData(lightColor, sizeof(vec3f) * lightCount, Buffer::Usage::STATIC_DRAW);
	
	Texture textureLightPositions(GL_TEXTURE_BUFFER);
	Texture textureLightColor(GL_TEXTURE_BUFFER);
	textureLightPositions.setType(Texture::Type::BUFFER);
	textureLightPositions.bind();
	textureLightPositions.attachBuffer(Type::VEC3F, bufferLightPosition.getName());
	textureLightColor.setType(Texture::Type::BUFFER);
	textureLightColor.bind();
	textureLightColor.attachBuffer(Type::VEC3F, bufferLightColor.getName());
	
	mesh->setTexture(textureLightPositions, 0);
	mesh->setTexture(textureLightColor, 1);
#else
	int32_t lightPositionLocation = program.getUniformLocation("lightPositions[0]");
	int32_t lightColorLocation = program.getUniformLocation("lightColors[0]");
	for(std::remove_const<decltype(lightCount)>::type i = 0; i < lightCount; ++i)
	{
		mesh->setUniform(lightPositionLocation + i, Type::VEC3F, lightPosition + i);
		mesh->setUniform(lightColorLocation + i, Type::VEC3F, lightColor + i);
	}
#endif

#if USE_TEXTURE
	std::string mapPath = FileSystem::getRelativePath("res/image/pbr/iron/");
	constexpr int32_t TYPE_COUNT = 5;
	using TT = Texture::Type;
	Texture::Type types[TYPE_COUNT] = {TT::ALBEDO, TT::AO, TT::ROUGHNESS, TT::METALLIC, TT::NORMAL};
	std::string typeNames[TYPE_COUNT] = {"albedo", "ao", "roughness", "metallic", "normal"};
	std::vector<std::shared_ptr<Texture>> textures;
	textures.reserve(TYPE_COUNT);
	Texture::Parameter parameter(10);
	for(int32_t i = 0; i < TYPE_COUNT; ++i)
	{
		std::string uniform = "texture" + typeNames[i];
		uniform[7] ^= 'a' - 'A';  // std::toupper
		int32_t location = program.getUniformLocation(uniform);
		std::shared_ptr<Texture> texture = std::make_shared<Texture>(GL_TEXTURE_2D);
		texture->setType(types[i]);
		bool flag = texture->load(mapPath + typeNames[i] + ".png");
		assert(flag);
		texture->setParameter(parameter);
		textures.push_back(std::move(texture));
		mesh->setTexture(*textures[i], 0);
	}
#else
	int32_t albedoLocation = program.getUniformLocation("albedo");
	int32_t metallicLocation = program.getUniformLocation("metallic");
	int32_t roughnessLocation = program.getUniformLocation("roughness");
	
	vec3f albedo(0.8, 0.4, 0.5);
	float metallic = 0.6;
	float roughness = 0.8;
	mesh->setUniform(albedoLocation, Type::VEC3F, &albedo);
	mesh->setUniform(metallicLocation, Type::FLOAT, &metallic);
	mesh->setUniform(roughnessLocation, Type::FLOAT, &roughness);
#endif

	mesh->setUniform(Shader::UNIFORM_MAT_MODEL, Type::MAT4F, &mesh->getTransform());
	mat4f projectionMatrix = Camera::perspective(M_PI / 3, 1, 1, 1024);
//	mat4f projectionMatrix = Camera::ortho(-2, +2, -2, +2, -2, +2);
	vec3f target(0, 0, 0);
	const vec3f up(0, 0, 1);
	float startTime = glfwGetTime();
	while(!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		float currentTime = glfwGetTime();
		float elapsedTime = currentTime - startTime;
		
		constexpr float radius = 2.0F;
		vec3f position(radius * std::cos(elapsedTime), radius * std::sin(elapsedTime), 2);
		mat4f viewMatrix = Camera::lookAt(position, target, up);
		mesh->setUniform(Shader::UNIFORM_VEC_VIEW_POSITION, Type::VEC3F, &position);
		mat4f viewProjection = Camera::multiply(viewMatrix, projectionMatrix);
		mesh->render(viewProjection);
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	glfwTerminate();
	return 0;
}
