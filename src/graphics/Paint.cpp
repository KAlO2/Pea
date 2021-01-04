#include "graphics/Paint.h"

#include <cassert>

#include "content/FontManager.h"
#include "opengl/TextureFont.h"
#include "util/unicode.h"

using namespace pea;

Paint::Paint():
		Paint(0U)
{
}

Paint::Paint(uint32_t flags):
		flags(flags),
		color(0, 0, 0, 0),
		textSize(12),
		textScaleX(1.0F),
		strokeWidth(5),
		alignment(Alignment::LEFT),
		typeface(nullptr)
{
//	typeface = FontManager::getInstance().getDefaultFont();
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

float Paint::measureText(const char32_t* text, size_t count) const
{
	TextureFont* font = FontManager::getInstance().getDefaultFont();
	return font->measureText(text, count, *this);
}

float Paint::measureText(const std::u32string& text) const
{
	TextureFont* font = FontManager::getInstance().getDefaultFont();
	return font->measureText(text, *this);
}

float Paint::measureText(const char* text, size_t count) const
{
	return measureText(std::string(text, count));
}

float Paint::measureText(const std::string& text) const
{
	std::u32string utf32 = codecvt_utf8_utf32(text);
	return measureText(utf32);
}
