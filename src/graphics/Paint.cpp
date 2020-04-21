#include "graphics/Paint.h"

#include <cassert>

using namespace pea;

Paint::Paint():
		color(0x0),
		textSize(18),
		textScaleX(1.0F),
		strokeWidth(5),
		alignment(Alignment::LEFT)
{
}
void Paint::setTextSize(float size)
{
	assert(size > 0);
	if(size > 0)
		this->textSize = size;
}

void Paint::setTextScaleX(float scale)
{
	assert(scale > 0);
	if(scale > 0)
		this->textScaleX = scale;
}
