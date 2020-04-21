#include "util/unit_test.h"

#include "geometry/algorithm.h"

#include <iostream>
#include <vector>
#include <random>

#include <GL/glut.h>
// use fixed function pipeline for drawing.

using Point = pea::vec2<float>;

static const size_t COUNT = 78;

void testSquareConvexHull()
{
	std::vector<Point> points;
	points.reserve(4 + COUNT);

	// construct a simple point set that it's apparently convex hull.
	const float R = 5.0;
	const Point quad2(-R, +R), quad1(+R, +R);  // +-+
	const Point quad3(-R, -R), quad4(+R, -R);  // +-+
	points.push_back(quad1);
	points.push_back(quad2);
	points.push_back(quad3);
	points.push_back(quad4);

	std::random_device device;
	std::mt19937 genarator(device());
	std::uniform_real_distribution<float> distrib(-R, +R);

	for(size_t i = 0; i < COUNT; ++i)
	{
		// generate points that in the square.
		Point point(distrib(genarator), distrib(genarator));
		points.push_back(point);
	}

	const std::vector<Point> hull = convexHull(points);

	REQUIRE(hull.size() == 5);
	REQUIRE(hull[0] == quad3);
	REQUIRE(hull[1] == quad4);
	REQUIRE(hull[2] == quad1);
	REQUIRE(hull[3] == quad2);
}

// global variables
Point top_right(8.0f, 6.0f);  // whatever positive value
std::vector<Point> points;

// [0, point.x] x [0, point.y]
// the coordinate of point must be positive value.
std::vector<Point> genRandomPoints(int width, int height)
{
	std::random_device device;
	std::mt19937 genarator(device());
	std::uniform_real_distribution<float> distrib(0, 1.0f);

	std::vector<Point> points(COUNT);
	for(size_t i = 0; i < COUNT; ++i)
	{
		float x = distrib(genarator) * width;
		float y = distrib(genarator) * height;
		points[i] = Point(x, y);
	}

	return points;
}

void onKeyboard(unsigned char key, int x, int y)
{
	// currently x and y are unused variables.
	std::ignore = x;
	std::ignore = y;
	
	switch(key)
	{
	case 27:  // ESC key
		exit(EXIT_SUCCESS);
		break;
	default:
		break;
	}
}

void onSpecial(int key, int x, int y)
{
	// currently x and y are unused variables.
	std::ignore = x;
	std::ignore = y;
	
	if(key == GLUT_KEY_F5) // press F5 to refresh
	{
		int width = glutGet(GLUT_WINDOW_WIDTH);
		int height = glutGet(GLUT_WINDOW_HEIGHT);
		points = genRandomPoints(width, height);
		glutPostRedisplay();
	}
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, static_cast<double>(width), 0.0, static_cast<double>(height));

	points = genRandomPoints(width, height);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glColor3f(1.0f, 1.0f, 0.0f); // point color: yellow
#if 0 // draw dots
	glBegin(GL_POINTS);
	for(const Point& point : points)
		glVertex2f(point.x, point.y);
	glEnd();
#else
	const float R = 5.0f;  // 5 pixels
	glBegin(GL_LINES);
	for(const Point& point : points)
	{
		glVertex2f(point.x - R, point.y);  // left
		glVertex2f(point.x + R, point.y);  // right
		glVertex2f(point.x, point.y - R);  // bottom
		glVertex2f(point.x, point.y + R);  // top
	}
	glEnd();
#endif

	const std::vector<Point> hull = convexHull(points);

	glColor3f(0.0f, 1.0f, 0.0f); // convex hull color: green
	glBegin(GL_LINE_STRIP);
	for(const Point& point : hull)
		glVertex2f(point.x, point.y);
	glEnd();

	glFlush();
}

void testGuiConvexHull()
{
	int argc = 1;
	glutInit(&argc, nullptr);

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Convex Hull");

	glutKeyboardFunc(onKeyboard);
	glutSpecialFunc(onSpecial);
	glutReshapeFunc(reshape);

	glClearColor(0, 0, 0, 0);
	glShadeModel(GL_FLAT);
/*
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, point.x, 0, point.y);
*/
	glutDisplayFunc(display);
	glutMainLoop();
}

int main()
{
	testSquareConvexHull();
	
	std::cout << "press F5 to refresh..." << std::endl;
	testGuiConvexHull();

	return 0;
}
