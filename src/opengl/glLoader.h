#ifndef PEA_OPENGL_GLLOADER_H_
#define PEA_OPENGL_GLLOADER_H_

// glew: http://glew.sourceforge.net/basic.html
// glad: https://glad.dav1d.de/
// --profile="core" --api="gl=4.5" --generator="c" --spec="gl" --extensions="GL_KHR_debug"
// http://glad.dav1d.de/#profile=core&language=c&specification=gl&loader=on&api=gl%3D4.5&extensions=GL_KHR_debug
#define GLEW_LOADER 1
#define GLAD_LOADER 2

#ifndef GL_FUNCTION_LOADER
#define GL_FUNCTION_LOADER GLEW_LOADER
#endif

#if GL_FUNCTION_LOADER == GLEW_LOADER
#  include <GL/glew.h>
#elif GL_FUNCTION_LOADER == GLEW_LOADER
#  include <glad/glad.h>
#else
#  error "missing OpenGL function loader"
#endif

// setup the OpenGL function pointers
inline bool loadGL()
{
#if GL_FUNCTION_LOADER == GLEW_LOADER
	glewExperimental = GL_TRUE;
	GLenum status = glewInit();
	return status == GLEW_OK;
#elif GL_FUNCTION_LOADER == GLAD_LOADER
	int status = gladLoadGL(glfwGetProcAddress);
	return status != 0;
#else

#endif
}

#endif  // PEA_OPENGL_GLLOADER_H_
