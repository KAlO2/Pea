#include "view/Window.h"

#include <algorithm>
#include <stdexcept>

#include "opengl/glLoader.h"  // glLoader.h must come before glfw3.h
#include <GLFW/glfw3.h>

#include "view/ViewRoot.h"
#include "util/Log.h"

static const char* TAG = "Window";

using namespace pea;

static GLFWwindow* createWindow(int32_t width, int32_t height, const char* title)
{
	// The counter part of glfwInit() is glfwTerminate().
	if(!glfwInit())  // needs to call glfwTerminate() after glfwInit successfully.
	{
//		slog.e(TAG, "GLFW failed to initialize");
		return nullptr;
	}
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 16);
	
	// GLFW window creation
	GLFWmonitor* monitor = nullptr;  // glfwGetPrimaryMonitor();
	GLFWwindow* window = glfwCreateWindow(width, height, title, monitor, nullptr/* share */);
	
	return window;
}

// https://stackoverflow.com/questions/21421074/how-to-create-a-full-screen-window-on-the-current-monitor-with-glfw
static GLFWmonitor* getCurrentMonitor(GLFWwindow *window)
{
	int32_t width, height, wx, wy;
	glfwGetWindowSize(window, &width, &height);
	glfwGetWindowPos(window, &wx, &wy);

	int32_t monitorCount;
	GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);
	int32_t bestOverlap = 0;
	GLFWmonitor* bestMonitor = nullptr;
	for(int32_t i = 0; i < monitorCount; ++i)
	{
		GLFWmonitor* monitor = monitors[i];
		const GLFWvidmode *mode = glfwGetVideoMode(monitor);
		int32_t mx, my;
		glfwGetMonitorPos(monitor, &mx, &my);
		
		int32_t overlap = std::max(0, std::min(wx + width, mx + mode->width) - std::max(wx, mx)) *
				std::max(0, std::min(wy + height, my + mode->height) - std::max(wy, my));
		if(overlap > bestOverlap)
		{
			bestOverlap = overlap;
			bestMonitor = monitor;
		}
	}
	
	return bestMonitor;
}


template <typename T>
static inline T* window_cast(GLFWwindow* window)
{
	return static_cast<T*>(glfwGetWindowUserPointer(window));
}

void Window::setCallbacks(GLFWwindow* window)
{
	glfwMakeContextCurrent(window);
	
	auto onWindowResize = [](GLFWwindow* window, int width, int height)
	{
		window_cast<Window>(window)->onWindowResize(width, height);
	};
	glfwSetFramebufferSizeCallback(window, onWindowResize);
	
	auto onMouseMove = [](GLFWwindow* window, double x, double y)
	{
		static bool firstTime = true;
		Window* _window = window_cast<Window>(window);
		if(!firstTime)
			_window->onMouseMove(x - _window->x, y - _window->y);
		else
			firstTime = false;
		
		_window->x = x;
		_window->y = y;
	};
	glfwSetCursorPosCallback(window, onMouseMove);
	
	auto onMouseScroll = [](GLFWwindow* window, double dx, double dy)
	{
		window_cast<Window>(window)->onMouseScroll(dx, dy);
	};
	glfwSetScrollCallback(window, onMouseScroll);
	
	auto onKey = [](GLFWwindow* window, int key, int scanCode, int action, int mods)
	{
		window_cast<Window>(window)->onKey(key, scanCode, action, mods);
	};
	glfwSetKeyCallback(window, onKey);
/*
	glfwSetCursorEnterCallback(window, onCursorEnter);
	glfwSetWindowFocusCallback(window, onWindowFocusChanged);
*/
//	glfwSetCursorPos(window, width/2.0, height/2.0);  // center of the window
//	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

Window::Window(int32_t width, int32_t height, const char* title):
		window(createWindow(width, height, title)),
		viewRoot(nullptr)
{
	if(window == nullptr)
		throw std::runtime_error("Failed to create GLFW window");
	else
	{
		slog.i(TAG, "create window@%p of specified size %dx%d", window, width, height);
		glfwSetWindowUserPointer(window, this);
		setCallbacks(window);
	}
	// load all OpenGL function pointers
	loadGL();
	GL::enableDebugMessage();
}

Window::Window(const char* title):
		window(nullptr),
		viewRoot(nullptr)
{
	// getCurrentMonitor()
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode *mode = glfwGetVideoMode(monitor);
	window = createWindow(mode->width, mode->height, title);
	
	if(window == nullptr)
		throw std::runtime_error("Failed to create GLFW window");
	else
	{
		slog.i(TAG, "create window@%p of fullscreen size %dx%d", window, mode->width, mode->height);
		glfwSetWindowUserPointer(window, this);
		setCallbacks(window);
	}
	
	// load all OpenGL function pointers
	loadGL();
	GL::enableDebugMessage();
}

Window::~Window()
{
	delete viewRoot;
	
	glfwTerminate();
}

void Window::onWindowResize(int32_t width, int32_t height)
{
	if(layoutVisibility != Visibility::GONE)
		updateUi(width, height);
}

void Window::updateUi(int32_t width, int32_t height)
{
	// update UI
	if(viewRoot == nullptr)
		viewRoot = new ViewRoot();
		
	viewRoot->setSize(width, height);
}

void Window::onKey(int32_t key, int32_t scanCode, int32_t action, int32_t modifierKeys)
{
	(void)key;
	(void)scanCode;
	(void)action;
	(void)modifierKeys;
}

void Window::onMouseMove(float dx, float dy)
{
	(void)dx;
	(void)dy;
}

void Window::onMouseScroll(float dx, float dy)
{
	(void)dx;
	(void)dy;
}

void Window::update(float dt)
{
	(void)dt;
}

void Window::render()
{
}

void Window::setTitle(const char* title)
{
	glfwSetWindowTitle(window, title);
}

void Window::setSize(int32_t width, int32_t height)
{
	assert(width > 0 && height > 0);
	glfwSetWindowSize(window, width, height);
}

vec2i Window::getSize() const
{
	int32_t width, height;
	glfwGetWindowSize(window, &width, &height);
	return vec2i(width, height);
}

void Window::setCursorVisibility(Visibility visibility)
{
	int32_t value;
	switch(visibility)
	{
	default:  assert(false);
	case Visibility::VISIBLE:    value = GLFW_CURSOR_NORMAL;   break;
	case Visibility::INVISIBLE:  value = GLFW_CURSOR_HIDDEN;   break;
	case Visibility::GONE:       value = GLFW_CURSOR_DISABLED; break;
	}
	glfwSetInputMode(window, GLFW_CURSOR, value);
}

Visibility Window::getCursorVisibility() const
{
	int32_t value = glfwGetInputMode(window, GLFW_CURSOR);
	switch(value)
	{
	default:  assert(false);
	case GLFW_CURSOR_NORMAL:    return Visibility::VISIBLE;
	case GLFW_CURSOR_HIDDEN:    return Visibility::INVISIBLE;
	case GLFW_CURSOR_DISABLED:  return Visibility::GONE;
	}
}

void Window::setLayoutVisibility(Visibility visibility)
{
	layoutVisibility = visibility;
	if(visibility == Visibility::GONE) [[unlikely]]
	{
		delete viewRoot;
		viewRoot = nullptr;
	}
	else  // VISIBLE or INVISIBLE
	{
		vec2i size = getSize();
		updateUi(size.width, size.height);
	}
}

Visibility Window::getLayoutVisibility() const
{
	return layoutVisibility;
}

vec2f Window::getCursorPosition() const
{
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	return vec2f(x, y);
}

void Window::run()
{
	double startTime = glfwGetTime();
	double currentTime = startTime;
	while(!glfwWindowShouldClose(window))
	{
		double lastTime = currentTime;
		currentTime = glfwGetTime();
		float time = currentTime - startTime;
		float dt = static_cast<float>(currentTime - lastTime);
		
		render();
		
		// draw 2D UI elements on the foreground, with depth test disabled.
		if(viewRoot != nullptr && layoutVisibility == Visibility::VISIBLE)
			viewRoot->draw();
			
		glfwSwapBuffers(window);
		
		glfwPollEvents();
		update(dt);
	}
}
