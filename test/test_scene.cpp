#define TAG "scene"
#include "scene/IndexBuffer.h"
#include "util/unit_test.h"
#include "util/GL.h"
#include <GL/glut.h>

using namespace pea;

void test_IndexBufer_class()
{
	IndexBuffer buffer;
	buffer.dump();
	printf("\n");
	buffer.create(vec2i(3, 2));
	buffer.dump();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(800, 600);
	glutCreateWindow(TAG);

	GLenum err = glewInit();
	if(err != GLEW_OK)
	{
		// Problem: glewInit failed, something is seriously wrong.
		slog.f(TAG, "Error: %s", glewGetErrorString(err));
		return EXIT_FAILURE;
	}
	if(!GLEW_VERSION_3_0)
	{
		slog.f(TAG, "No support for OpenGL 3.0 found");
		return EXIT_FAILURE;
	}
	slog.i(TAG, "Status: Using OpenGL %s", glGetString(GL_VERSION));
	slog.i(TAG, "Status: Using GLEW %s", glewGetString(GLEW_VERSION));

	test_IndexBufer_class();


	return 0;
}

