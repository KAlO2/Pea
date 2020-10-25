#ifndef PEA_VIEW_WINDOW_H_
#define PEA_VIEW_WINDOW_H_

#include <cstdint>

#include "math/vec2.h"
#include "view/Visibility.h"

class GLFWwindow;

namespace pea {

class ViewRoot;

/**
 * Previously, I write singleton pattern like below:
 * @code
 * auto onWindowResize = [](GLFWwindow* window, int width, int height)
 * {
 *     Window& window = Window::getInstance();
 *     window::onWindowResize(width, height);
 * };
 * glfwSetFramebufferSizeCallback(window, onWindowResize);
 * @endcode
 *
 * What does glfwGetWindowUserPointer do?
 * A UserData field is a fairly common paradigm in C APIs that lets the user access contextual data 
 * from within callbacks without needing to make everything global. In essence, it lets you 
 * associate an arbitrary piece of data relevant to your program with a glfw window.
 *
 * If you are trying to use glfw in a program that follows object-oriented design, for example, you 
 * can use this pointer to store the address of the instance that is handling a particular window, 
 * and forward the callbacks (which have to be static functions, because of the way the API works) 
 * to the appropriate member functions.
 *
 * @see https://stackoverflow.com/questions/5008266/callback-function-in-freeglut-from-object
 * @see https://stackoverflow.com/questions/7676971/pointing-to-a-function-that-is-a-class-member-glfw-setkeycallback
 * @see http://discourse.glfw.org/t/1294
 */
class Window
{
protected:
	GLFWwindow* window;
	float framePerSecond = 60;
	
	ViewRoot* viewRoot;
	Visibility layoutVisibility;
	
	double x, y;

private:
	static void setCallbacks(GLFWwindow* window);
	
	void updateUi(int32_t width, int32_t height);
protected:
	/**
	 * Whenever the window size changed (by OS or user resize), this callback function executes.
	 */
	virtual void onWindowResize(int32_t width, int32_t height);
	
	/**
	 * Called when a key is pressed, repeated or released.
	 */
	virtual void onKey(int32_t key, int32_t scanCode, int32_t action, int32_t modifierKeys);
	
	/**
	 * Whenever the mouse moves, this callback is called. top left coordinate is origion (0, 0)
	 *
	 */
	virtual void onMouseMove(float dx, float dy);
	
	/**
	 * whenever the mouse scroll wheel scrolls, this callback is called
	 */
	virtual void onMouseScroll(float dx, float dy);
	
	virtual void render();
	
	virtual void update(float dt);
	
public:
	/**
	 * create a fullscreen window
	 */
	explicit Window(const char* title);
	
	/**
	 * create a window with specified size.
	 */
	Window(int32_t width, int32_t height, const char* title);
	
	virtual ~Window();
	
	Window(const Window& window) = delete;
	Window& operator =(const Window& window) = delete;
	
	void setTitle(const char* title);
	
	void setSize(int32_t width, int32_t height);
	vec2i getSize() const;
	
	void setCursorVisibility(Visibility visibility);
	Visibility getCursorVisibility() const;

	void setLayoutVisibility(Visibility visibility);
	Visibility getLayoutVisibility() const;
	
	vec2f getCursorPosition() const;
	
	ViewRoot* getViewRoot();
	
	void run();
};

inline ViewRoot* Window::getViewRoot() { return viewRoot; }

}  // namespace pea
#endif  // PEA_VIEW_WINDOW_H_
