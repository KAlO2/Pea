#include "graphics/Canvas.h"
#include "graphics/Image.h"

using namespace pea;


vec2i Canvas::getSize() const
{
	int32_t width = bitmap->getWidth();
	int32_t height = bitmap->getHeight();
	return vec2i(width, height);
}

void Canvas::translate(float dx, float dy)
{
	transform.translate(vec2(dx, dy));
}

void Canvas::rotate(float angle)
{
	transform.rotate(angle);
}

void Canvas::rotate(float angle, float px, float py)
{
	translate(px, py);
	rotate(angle);
	translate(-px, -py);
}

void Canvas::scale(float sx, float sy)
{
	transform.scale(vec2f(sx, sy));
}

void Canvas::scale(float sx, float sy, float px, float py)
{
	translate(px, py);
	scale(sx, sy);
	translate(-px, -py);
}

void Canvas::skew(float sx, float sy)
{
	transform.skew(vec2f(sx, sy));
}

void Canvas::setTransform(const mat3f& transform)
{
	this->transform = transform;
}

const mat3f& Canvas::getTransform() const
{
	return transform;
}

void Canvas::clipRect(const Rect<int32_t>& region)
{
}

void Canvas::drawRect(const Rect<int32_t>& rect, const Paint& paint)
{
}

void Canvas::drawRect(const Rect<float>& rect, const Paint& paint)
{
}

void Canvas::drawCircle(float x, float y, float radius, const Paint& paint)
{
}

void Canvas::drawText(const char* text, size_t count, const vec2f& position, const Paint& paint)
{
}

