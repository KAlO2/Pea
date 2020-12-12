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
	assert(format != Color::Format::UNKNOWN);
	// In OpenGL 3 the GL_LUMINANCE and GL_ALPHA texture internal formats are deprecated, are 
	// not available in core profile. They have been replaced by the GL_RED texture format.
//	return GL_LUMINANCE;
//	return GL_LUMINANCE_ALPHA;
	uint32_t c = Color::sizeofChannel(format) - 1;  // make it zero indexed
	GLenum pixelFormats_f[4] = {GL_RED, GL_RG, GL_RGB, GL_RGBA};
	return pixelFormats_f[c];
/*
	GLenum pixelFormats_i[4] = {GL_RED_INTEGER, GL_RG_INTEGER, GL_RGB_INTEGER, GL_RGBA_INTEGER};
	if(Color::isFloatType(format))
		return pixelFormats_f[c];
	else
		return pixelFormats_i[c];
*/
}

GLenum GL::dataType(Color::Format format)
{
	// Are the pixel types GL_UNSIGNED_BYTE and GL_UNSIGNED_INT_8_8_8_8 fully equivalent?
	// https://stackoverflow.com/questions/7786187/opengl-texture-upload-unsigned-byte-vs-unsigned-int-8-8-8-8
	// for RGBA, Color::Format put R the first byte, while OpenGL put R the highest weight, so 
	// we use _REV (reverse)to revise the data. 
	switch(format)
	{
	case Color::C1_U8:
	case Color::C2_U8:
	case Color::C3_U8:
	case Color::C4_U8:
		return GL_UNSIGNED_BYTE;
		
	case Color::C1_U16:
	case Color::C2_U16:
	case Color::C3_U16:
	case Color::C4_U16:
		return GL_UNSIGNED_SHORT;
	
	case Color::C1_I16:
	case Color::C2_I16:
	case Color::C3_I16:
	case Color::C4_I16:
		return GL_SHORT;
	
	case Color::C1_U32:
		return GL_UNSIGNED_INT;
	
	case Color::C1_I32:
		return GL_INT;
	
	case Color::C1_F16:
		return GL_HALF_FLOAT;
	
	case Color::C1_F32:
		return GL_FLOAT;
		
	case Color::RGB565_U16:
		return GL_UNSIGNED_SHORT_5_6_5_REV;
		
	case Color::RGBA5551_U16:
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
	case GL_RED_INTEGER:
	case GL_RED:
		return 1;
	
	case GL_RG_INTEGER:
	case GL_RG:
		return 2;
	
	case GL_BGR_INTEGER:
	case GL_RGB_INTEGER:
	case GL_BGR:
	case GL_RGB:
		return 3;
	
	case GL_BGRA_INTEGER:
	case GL_RGBA_INTEGER:
	case GL_BGRA:
	case GL_RGBA: return 4;
	
	default:
		assert(false);
		return 0;
	}
}

int32_t GL::align(int32_t width, Color::Format colorFormat)
{
	int32_t alignment = 1;
	for(int32_t rowStride = width * Color::size(colorFormat); (rowStride & 1) == 0; rowStride >>= 1)
	{
		alignment <<= 1;
		
		// The allowable alignment values are 1, 2, 4, 8
		if(alignment >= 8)
			break;
	}
	
	return alignment;
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

#endif
