#ifndef PEA_OPENGL_UTILITY_H_
#define PEA_OPENGL_UTILITY_H_

/*
#ifdef GL_GLEXT_LEGACY
#undef GL_GLEXT_LEGACY
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
*/

// Use glew.h instead of gl.h to get all the GL prototypes declared
#if defined(_WIN32) || defined(_WIN64)
#  include <GL/glew.h>
#elif defined(__APPLE__)
#  include "TargetConditionals.h"
#    if TARGET_OS_SIMULATOR || TARGET_OS_IPHONE
#      if defined(FREETYPE_GL_ES_VERSION_3_0)
#        include <OpenGLES/ES3/gl.h>
#      else
#        include <OpenGLES/ES2/gl.h>
#    endif
#  else
#    include <OpenGL/gl.h>
#  endif
#else
#  include <GL/glew.h>
//#  include <GL/glu.h>
#endif

#include <vector>

#include "graphics/Color.h"
#include "geometry/Primitive.h"
#include "math/vec2.h"
#include "math/vec3.h"
#include "math/vec4.h"
#include "math/mat4.h"

// drop deprecated functions like glBegin/glEnd in favor of programmable pipeline
#ifdef NDEBUG
#define GL_CHECK_ERROR(...) (void)0
#else
#define GL_CHECK_ERROR(...)                                                                \
	do                                                                                     \
	{                                                                                      \
	    bool found = GL::checkError();                                                     \
	    if(found)                                                                          \
	        slog.e("OpenGL", "error in %s::%s @%+d\n", __FILE__, __FUNCTION__, __LINE__);  \
	} while(false)
#endif

namespace pea {

/**
 * @brief a wrapper class for OpenGL
 *
 * @see https://www.khronos.org/opengl/wiki/Debugging_Tools
 *
 * precision
 * column-major
 * right hand coordinate
 * use radian measure instead of degree measure
 */
class GL
{
private:

	enum Capability: uint32_t
	{
		LIGHTING  = GL_LIGHTING,
		CULL_FACE = GL_CULL_FACE,
	};
	
public:

	/**
	 * Checks for OpenGL errors and prints it.
	 * @return Whether or not any errors were found.
	 */
	static bool checkError();

	/**
	 * Enable OpenGL debug message. And no messages will be logged in release mode.
	 * Note that catching GL errors with this method is much easier than by using glGetError() error
	 * checking, as there is no need to sprinkle glGetError() calls throughout the code.
	 *
	 * @see https://www.khronos.org/opengl/wiki/Debug_Output
	 */
	static void enableDebugMessage();
	
	static bool checkExtension(const std::string& extension);
	
	template<class T>
	struct dependent_false: std::false_type
	{
	};

	/**
	 * @param[in] target
	 * @param[in] index vertex attribute index
	 */
	template <typename T>
	static inline void bindBuffer(const uint32_t& vbo, GLenum target, const T* buffer, int32_t length, GLenum usage = GL_STATIC_DRAW)
	{
		glBindBuffer(target, vbo);
		glBufferData(target, length * sizeof(T), buffer, usage);
	}
	
	template <typename T>
	static uint32_t bindVertexBuffer(const uint32_t& vbo, GLuint index, const T* buffer, int32_t length, GLenum usage = GL_STATIC_DRAW)
	{
		bindBuffer(vbo, GL_ARRAY_BUFFER, buffer, length, usage);

		glEnableVertexAttribArray(index);
		GLint size;
		GLenum type;
		     if constexpr(std::is_same<T, vec4f>::value)    { size = 4; type = GL_FLOAT; }
		else if constexpr(std::is_same<T, vec3f>::value)    { size = 3; type = GL_FLOAT; }
		else if constexpr(std::is_same<T, vec2f>::value)    { size = 2; type = GL_FLOAT; }
		else if constexpr(std::is_same<T, float>::value)    { size = 1; type = GL_FLOAT; }
		
		else if constexpr(std::is_same<T, vec4b>::value)    { size = 4; type = GL_UNSIGNED_BYTE;  }
		else if constexpr(std::is_same<T, uint8_t>::value)  { size = 1; type = GL_UNSIGNED_BYTE;  }
		else if constexpr(std::is_same<T, uint16_t>::value) { size = 1; type = GL_UNSIGNED_SHORT; }
		else if constexpr(std::is_same<T, uint32_t>::value) { size = 1; type = GL_UNSIGNED_INT;   }
		
		else static_assert(dependent_false<T>::value, "invalid type");
		// static_assert(false) will always trigger compile-time error,
		// The common workaround for such case is a type-dependent expression that is always false.
		
		glVertexAttribPointer(index, size, type, GL_FALSE, sizeof(T), (void*)0);
		return vbo;
	}
	
	template <typename T>
	static void bindIndexBuffer(const uint32_t& vbo, const T* buffer, int32_t length)
	{
		bindBuffer(vbo, GL_ELEMENT_ARRAY_BUFFER, buffer, length);
	}
	
	template <typename T>
	static void bindUniformBuffer(const uint32_t& ubo, const T* buffer, int32_t length)
	{
		bindBuffer(ubo, GL_UNIFORM_BUFFER, buffer, length);
	}
	
	template <typename T>
	static inline void bindVertexBuffer(const uint32_t& vbo, GLuint index, const std::vector<T>& buffer, GLenum usage = GL_STATIC_DRAW)
	{
		bindVertexBuffer(vbo, index, buffer.data(), buffer.size(), usage);
	}
	
	template <typename T>
	static inline void bindIndexBuffer(const uint32_t& vbo, const std::vector<T>& buffer)
	{
		bindIndexBuffer(vbo, buffer.data(), buffer.size());
	}
	
	
	static GLenum pixelFormat(Color::Format format);
	static GLenum dataType(Color::Format format);
	static GLenum getPrimitive(Primitive primitive);
	
	// $ glxinfo
//	static void displayVersionInfo();
	
	static const char* glslTypeToString(GLenum type);
	
/*
	static uint32_t glCast(Buffer::Usage usage);
	static uint32_t glCast(Buffer::Type type);
	static uint32_t glCast(Data::Type type);
	static uint32_t glCast(Mesh::Primitive primitive);
*/

#if 0
	/**
	 * Draws a cross on the screen. Will be in whatever color you last passed in
	 * to a glColor* function.
	 *
	 * @param point The point for the center of the cross (in world space)
	 * @param size How long each arm should be (in GL units)
	 */
	static void drawCross(const vec2f& center, float radius);
	static void drawAxis(float length);

	static void drawCircle(const vec2f& center, float radius, bool solid = false);
	static void drawCircle(const vec3f& center, float radius, const vec3f& normal, bool solid = false);
	static void draw(const Triangle& triangle);
	static void draw(const Rectangle& rect);
	static void draw(const AxisAlignedBox& aabb);
	static void draw(const Cylinder& cylinder);
#endif

};

// man glBindAttribLocation for matrix attribute variable.
// mat2f can be changed to vec4f, which takes only one slot.
//template <>
//uint32_t GL::bindVertexBuffer<mat2f>(const uint32_t& vbo, GLuint index, const mat2f* buffer, int32_t length, GLenum usage/* = GL_STATIC_DRAW*/);

template <>
uint32_t GL::bindVertexBuffer<mat3f>(const uint32_t& vbo, GLuint index, const mat3f* buffer, int32_t length, GLenum usage/* = GL_STATIC_DRAW*/);

template <>
uint32_t GL::bindVertexBuffer<mat4f>(const uint32_t& vbo, GLuint index, const mat4f* buffer, int32_t length, GLenum usage/* = GL_STATIC_DRAW*/);

}  // namespace pea
#endif  // PEA_OPENGL_UTILITY_H_
