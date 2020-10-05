#ifndef PEA_EXAMPLE_COMMON_H_
#define PEA_EXAMPLE_COMMON_H_

#include "opengl/glLoader.h"
#include <GLFW/glfw3.h>

// common headers
#include "scene/Camera.h"
#include "opengl/GL.h"
#include "opengl/Program.h"
#include "opengl/Shader.h"
#include "opengl/ShaderFactory.h"
#include "opengl/Texture.h"
#include "util/Log.h"

namespace pea {

inline vec2i getWindowSize(GLFWwindow* window)
{
	int32_t width, height;
	glfwGetWindowSize(window, &width, &height);
	return vec2i(width, height);
}

inline vec2i getMousePosition(GLFWwindow* window)
{
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	// mouse position is integers.
	return vec2i(x, y);
}

// GLFW: whenever the window size changes (by OS or user resize), this callback function executes.
void onWindowResize(GLFWwindow* window, int width, int height);

/**
 * @return time in milliseconds
 */
int64_t getCurrentTime();

/**
 * format time into YYmmdd-HHMMSS, i.e. 20200319-155806
 */
std::string formatCurrentTime();

void snapshot(const std::string& path, int32_t width, int32_t height);

Texture createThermosTexture(uint32_t width);

}  // namespace pea
#endif  // PEA_EXAMPLE_COMMON_H_
