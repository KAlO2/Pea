#include "opengl/GL.h"

#include <string>
#include <string.h>

#include "graphics/Color.h"
#include "util/Log.h"
#include "util/utility.h"


static const char* TAG = "OpenGL";

using namespace pea;

bool GL::checkError()
{
	bool found = false;
	GLenum code = GL_NO_ERROR;
	do
	{
/*
		If more than one flag has recorded an error, glGetError returns and
		clears an arbitrary error flag value. Thus, glGetError should always
		be called in a loop, until it returns GL_NO_ERROR, if all error flags
		are to be reset.
*/
		code = glGetError();
		if(code != GL_NO_ERROR)
		{
			found = true;
			slog.e(TAG, "Error: %s", gluErrorString(code));
		}
	} while(code != GL_NO_ERROR);

	return found;
}

static void debugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
#define CASE(CATEGORY, E)  case CATEGORY##E: return #E

	auto humanReadableSource = [](GLenum source) -> const char*
	{
		switch(source)
		{
#define GL_DEBUG_SOURCE_CASE(E)      CASE(GL_DEBUG_SOURCE_, E)
		GL_DEBUG_SOURCE_CASE(API);
		GL_DEBUG_SOURCE_CASE(WINDOW_SYSTEM);
		GL_DEBUG_SOURCE_CASE(SHADER_COMPILER);
		GL_DEBUG_SOURCE_CASE(THIRD_PARTY);
		GL_DEBUG_SOURCE_CASE(APPLICATION);
		GL_DEBUG_SOURCE_CASE(OTHER);
#undef GL_DEBUG_SOURCE_CASE
		default: return "GL_DEBUG_SOURCE_???";
		}
	};
	
	auto humanReadableType = [](GLenum type) -> const char*
	{
		switch(type)
		{
#define GL_DEBUG_TYPE_CASE(E)      CASE(GL_DEBUG_TYPE_, E)
		GL_DEBUG_TYPE_CASE(ERROR);
		GL_DEBUG_TYPE_CASE(DEPRECATED_BEHAVIOR);
		GL_DEBUG_TYPE_CASE(UNDEFINED_BEHAVIOR);
		GL_DEBUG_TYPE_CASE(PORTABILITY);
		GL_DEBUG_TYPE_CASE(PERFORMANCE);
		GL_DEBUG_TYPE_CASE(MARKER);
		GL_DEBUG_TYPE_CASE(PUSH_GROUP);
		GL_DEBUG_TYPE_CASE(POP_GROUP);
		GL_DEBUG_TYPE_CASE(OTHER);
#undef GL_DEBUG_TYPE_CASE
		default: return "GL_DEBUG_TYPE_???";
		}
	};
	
	auto humanReadableSeverity = [](GLenum severity) -> const char*
	{
		switch(severity)
		{
#define GL_DEBUG_SEVERITY_CASE(E)  CASE(GL_DEBUG_SEVERITY_, E)
		GL_DEBUG_SEVERITY_CASE(HIGH);
		GL_DEBUG_SEVERITY_CASE(MEDIUM);
		GL_DEBUG_SEVERITY_CASE(LOW);
		GL_DEBUG_SEVERITY_CASE(NOTIFICATION);
#undef GL_DEBUG_SEVERITY_CASE
		default: return "GL_DEBUG_SEVERITY_???";
		}
	};

#undef CASE

	slog.e(TAG, "GL callback: source=%s, type=%s, severity=%s, message=\"%s\"",
			humanReadableSource(source), humanReadableType(type), humanReadableSeverity(severity), message);
	
	// to suppress unused parameter warnings
	(void)id;  // id=%u, id​ is the message's identifier integer
	(void)length;  // length=%zu, length​ is the length of the message​ string, excluding the null-terminator.
	(void)userParam;
}

void GL::enableDebugMessage()
{
#ifndef NDEBUG
	glEnable(GL_DEBUG_OUTPUT);
//	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	void* userParam = nullptr;  // userParam is not used here.
	glDebugMessageCallback(debugMessage, userParam);
	
	// filter out certain message if you need.
	// https://www.khronos.org/opengl/wiki/GLAPI/glDebugMessageControl
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0/*count*/, nullptr/*ids​*/, false/*enabled​*/);
#endif
}

bool GL::checkExtension(const std::string& extension)
{
	static const std::string all(reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)));

//	we can't distinguish GL_ARB_texture_cube_map from GL_ARB_texture_cube_map_array with this line.
//	all.find(extension) != std::string::npos;  // WRONG code

	const size_t length = extension.length();
	size_t i = all.find(extension);
	while(i != std::string::npos)
	{
		if(!isalnum(all[i + length]))
			return true;
	}

	return false;
}

template <>
uint32_t GL::bindVertexBuffer<mat3f>(const uint32_t& vbo, GLuint index, const mat3f* buffer, int32_t length, GLenum usage/* = GL_STATIC_DRAW*/)
{
	bindBuffer(vbo, GL_ARRAY_BUFFER, buffer, length, usage);

	// a mat3 attribute takes up 3 attribute locations
	for(uint32_t i = 0; i < 3; ++i)
	{
		glEnableVertexAttribArray(index + i);
		size_t offset = sizeof(vec3f) * i;
		glVertexAttribPointer(index + i, 3, GL_FLOAT, GL_FALSE, sizeof(mat3f), (void*)offset);
	}
	
	return vbo;
}

template <>
uint32_t GL::bindVertexBuffer<mat4f>(const uint32_t& vbo, GLuint index, const mat4f* buffer, int32_t length, GLenum usage/* = GL_STATIC_DRAW*/)
{
	bindBuffer(vbo, GL_ARRAY_BUFFER, buffer, length, usage);

	// a mat4 attribute takes up 4 attribute locations
	for(uint32_t i = 0; i < 4; ++i)
	{
		glEnableVertexAttribArray(index + i);
		size_t offset = sizeof(vec4f) * i;
		glVertexAttribPointer(index + i, 4, GL_FLOAT, GL_FALSE, sizeof(mat4f), (void*)offset);
	}
	
	return vbo;
}

GLenum GL::pixelFormat(Color::Format format)
{
	switch(format)
	{
	case Color::G_8:
		// In OpenGL 3 the GL_LUMINANCE and GL_ALPHA texture internal formats are deprecated, are 
		// not available in core profile. They have been replaced by the GL_RED texture format.
//		return GL_LUMINANCE;
		return GL_RED;
	case Color::GA_88:
		return GL_RG;
//		return GL_LUMINANCE_ALPHA;
	case Color::RGB_565:
	case Color::RGB_888:
		return GL_RGB;
	case Color::BGR_888:
		return GL_BGR;
	case Color::RGBA_5551:
	case Color::RGBA_8888:
		return GL_RGBA;
	case Color::BGRA_8888:
		return GL_BGRA;
	default:
		assert(false);
		return 0;
	}
}

GLenum GL::dataType(Color::Format format)
{
	// Are the pixel types GL_UNSIGNED_BYTE and GL_UNSIGNED_INT_8_8_8_8 fully equivalent?
	// https://stackoverflow.com/questions/7786187/opengl-texture-upload-unsigned-byte-vs-unsigned-int-8-8-8-8
	// for RGBA, Color::Format put R the first byte, while OpenGL put R the highest weight, so 
	// we use _REV (reverse)to revise the data. 
	switch(format)
	{
	case Color::G_8:
	case Color::GA_88:
	case Color::RGB_888:
	case Color::BGR_888:
	case Color::RGBA_8888:
	case Color::BGRA_8888:
		return GL_UNSIGNED_BYTE;
		
	case Color::RGB_565:
		return GL_UNSIGNED_SHORT_5_6_5_REV;
		
	case Color::RGBA_5551:
		return GL_UNSIGNED_SHORT_5_5_5_1;
		
	default:
		assert(false);
		return 0;
	}
}

uint32_t GL::sizeofChannel(GLint format)
{
	switch(format)
	{
	case GL_RED:  return 1;
	case GL_RG:   return 2;
	case GL_BGR:
	case GL_RGB:  return 3;
	case GL_BGRA:
	case GL_RGBA: return 4;
	
	default:
		assert(false);
		return 0;
	}
}

/*
void GL::displayVersionInfo()
{
	std::cout << "OpenGL version:  " << glGetString(GL_VERSION) << '\n'
			<< "OpenGL vendor:   " << glGetString(GL_VENDOR) << '\n'
			<< "GLSL version:    " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n'
			<< "OpenGL renderer: " << glGetString(GL_RENDERER) << '\n';
}
*/
const char* GL::glslTypeToString(GLenum type)
{
	switch(type)
	{
	case GL_FLOAT:       return "float";
	case GL_FLOAT_VEC2:  return "vec2";
	case GL_FLOAT_VEC3:  return "vec3";
	case GL_FLOAT_VEC4:  return "vec4";
	
	case GL_FLOAT_MAT2:  return "mat2";
	case GL_FLOAT_MAT3:  return "mat3";
	case GL_FLOAT_MAT4:  return "mat4";
	case GL_FLOAT_MAT2x3:return "mat2x3";
	case GL_FLOAT_MAT2x4:return "mat2x4";
	case GL_FLOAT_MAT3x2:return "mat3x2";
	case GL_FLOAT_MAT3x4:return "mat3x4";
	case GL_FLOAT_MAT4x2:return "mat4x2";
	case GL_FLOAT_MAT4x3:return "mat4x3";
	
	case GL_INT:         return "int";
	case GL_INT_VEC2:    return "ivec2";
	case GL_INT_VEC3:    return "ivec3";
	case GL_INT_VEC4:    return "ivec4";
	
	case GL_UNSIGNED_INT:       return "uint";
	case GL_UNSIGNED_INT_VEC2:  return "uint2";
	case GL_UNSIGNED_INT_VEC3:  return "uint3";
	case GL_UNSIGNED_INT_VEC4:  return "uint4";
	
	case GL_BOOL:         return "bool";
	case GL_BOOL_VEC2:    return "bvec2";
	case GL_BOOL_VEC3:    return "bvec3";
	case GL_BOOL_VEC4:    return "bvec4";
	
	case GL_DOUBLE:      return "double";
	case GL_DOUBLE_VEC2: return "dvec2";
	case GL_DOUBLE_VEC3: return "dvec3";
	case GL_DOUBLE_VEC4: return "dvec4";
	case GL_DOUBLE_MAT2: return "dmat2";
	case GL_DOUBLE_MAT3: return "dmat3";
	case GL_DOUBLE_MAT4: return "dmat4";
	case GL_DOUBLE_MAT2x3:  return "dmat2x3";
	case GL_DOUBLE_MAT2x4:  return "dmat2x4";
	case GL_DOUBLE_MAT3x2:  return "dmat3x2";
	case GL_DOUBLE_MAT3x4:  return "dmat3x4";
	case GL_DOUBLE_MAT4x2:  return "dmat4x2";
	case GL_DOUBLE_MAT4x3:  return "dmat4x3";
	
	case GL_SAMPLER_1D:  return "sampler1D";
	case GL_SAMPLER_2D:  return "sampler2D";
	case GL_SAMPLER_2D_RECT:  return "sample2DRect";
	case GL_SAMPLER_3D:  return "sampler3D";
	case GL_SAMPLER_CUBE:return "samplerCube";

	default:             return "???";
	}
}
#if 0
uint32_t GL::glCast(Buffer::Usage usage)
{
/*
#define GL_STREAM_DRAW                    0x88E0
#define GL_STREAM_READ                    0x88E1
#define GL_STREAM_COPY                    0x88E2

#define GL_STATIC_DRAW                    0x88E4
#define GL_STATIC_READ                    0x88E5
#define GL_STATIC_COPY                    0x88E6

#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_DYNAMIC_READ                   0x88E9
#define GL_DYNAMIC_COPY                   0x88EA
*/
	uint32_t offset = static_cast<uint32_t>(usage);
	assert(offset <= 0xA && offset != 3 && offset != 7);
	return GL_STREAM_DRAW + offset;
}

uint32_t GL::glCast(Buffer::Type type)
{
	switch(type)
	{
	case Buffer::Type::VERTEX:  return GL_ARRAY_BUFFER;
	case Buffer::Type::INDEX:   return GL_ELEMENT_ARRAY_BUFFER;
	case Buffer::Type::TEXTURE: return GL_TEXTURE_BUFFER;
	case Buffer::Type::UNIFORM: return GL_UNIFORM_BUFFER;
	default: assert(false);     return 0;
	}
}

uint32_t GL::glCast(Data::Type type)
{
/*
#define GL_BYTE                 0x1400
#define GL_UNSIGNED_BYTE        0x1401
#define GL_SHORT                0x1402
#define GL_UNSIGNED_SHORT       0x1403
#define GL_INT                  0x1404
#define GL_UNSIGNED_INT         0x1405
#define GL_FLOAT                0x1406
#define GL_2_BYTES              0x1407
#define GL_3_BYTES              0x1408
#define GL_4_BYTES              0x1409
#define GL_DOUBLE               0x140A
#define GL_HALF_FLOAT           0x140B
*/
	switch(type)
	{
	case Data::TYPE_BYTE:   return GL_BYTE;
	case Data::TYPE_UBYTE:  return GL_UNSIGNED_BYTE;
	case Data::TYPE_SHORT:  return GL_SHORT;
	case Data::TYPE_USHORT: return GL_UNSIGNED_SHORT;
	case Data::TYPE_INT:    return GL_INT;
	case Data::TYPE_UINT:   return GL_UNSIGNED_INT;
	case Data::TYPE_HALF:   return GL_HALF_FLOAT;
	case Data::TYPE_FLOAT:  return GL_FLOAT;
	case Data::TYPE_DOUBLE: return GL_DOUBLE;
	default: assert(false); return GL_BYTE;
	}
}

uint32_t GL::glCast(Mesh::Primitive primitive)
{
/*
#define GL_POINTS               0x0000
#define GL_LINES                0x0001
#define GL_LINE_LOOP            0x0002
#define GL_LINE_STRIP           0x0003
#define GL_TRIANGLES            0x0004
#define GL_TRIANGLE_STRIP       0x0005
#define GL_TRIANGLE_FAN         0x0006
#define GL_QUADS                0x0007
#define GL_QUAD_STRIP           0x0008
#define GL_POLYGON              0x0009
*/
	return static_cast<uint32_t>(primitive);
}

void GL::drawCross(const vec2f &center, float radius)
{
	const vec2f point[] =
	{
		vec2f(center.x - radius, center.y),  // left
		vec2f(center.x + radius, center.y),  // right
		vec2f(center.x, center.y - radius),  // bottom
		vec2f(center.x, center.y + radius),  // top
	};

#if USE_GL_VERTEX_ARRAY
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, point);
	glDrawArrays(GL_LINES, 0, 4);
#else
	// type(point) is vec2f*, while type(&points) is vec2f (*)[4].
	glBegin(GL_LINES);
	glVertex2fv(reinterpret_cast<const float*>(point));
	glVertex2fv(reinterpret_cast<const float*>(point + 1));
	glVertex2fv(reinterpret_cast<const float*>(point + 2));
	glVertex2fv(reinterpret_cast<const float*>(point + 3));
	glEnd();
#endif
}

#endif
