#include "graphics/Color.h"

#include <cstring>

#include "math/scalar.h" // for clamp

using namespace pea;
/*
const Color Color::RED     = Color(255,   0,   0);
const Color Color::GREEN   = Color(  0, 255,   0);
const Color Color::BLUE    = Color(  0,   0, 255);
const Color Color::BLACK   = Color(  0,   0,   0);
const Color Color::WHITE   = Color(255, 255, 255);
const Color Color::CYAN    = Color(  0, 255, 255);
const Color Color::MAGENTA = Color(255,   0, 255);
const Color Color::YELLOW  = Color(255, 255,   0);
*/
uint32_t Color::rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a/* = 255 */)
{
	return r | (g << 8) | (b << 16) | (a << 24);
}

uint32_t Color::from_G8(uint8_t x)
{
	return x | (x << 8) | (x << 16) | 0xFF000000;
}

uint32_t Color::from_GA88(uint16_t x)
{
	uint8_t g = static_cast<uint8_t>(x);
	uint8_t a = static_cast<uint8_t>(x >> 8);
	return g | (g << 8) | (g << 16) | (a << 24);
}

uint32_t Color::from_RGBX5551(uint16_t x)
{
/*
	uint8_t r = static_cast<uint8_t>();
	uint8_t g = static_cast<uint8_t>((x & 0x03E0) << 3);
	uint8_t b = static_cast<uint8_t>((x & 0x7C00) << 3);
	return Color(r, g, b);
*/
	return ((x & 0x001F) << 3) | ((x & 0x03E0) << 6) | ((x & 0x7C00) << 9) | 0xFF000000;
}

uint32_t Color::from_RGBA5551(uint16_t x)
{
/*
	uint8_t r = static_cast<uint8_t>((x & 0x001f) << 3);
	uint8_t g = static_cast<uint8_t>((x & 0x03e0) << 3);
	uint8_t b = static_cast<uint8_t>((x & 0x7c00) << 3);
	uint8_t a = static_cast<uint8_t>((x & 0x8000) << 7);
	return Color(r, g, b, a);
*/
	uint32_t color = ((x & 0x001F) << 3) | ((x & 0x03E0) << 6) | ((x & 0x7C00) << 9);
	if((x & 0x8000) != 0)
		color |= 0xFF000000;
	return color;
}

uint32_t Color::from_RGBA4444(uint16_t x)
{
/*
	uint8_t r = static_cast<uint8_t>((x & 0x000f) << 4);
	uint8_t g = static_cast<uint8_t>((x & 0x00f0) << 4);
	uint8_t b = static_cast<uint8_t>((x & 0x0f00) << 4);
	uint8_t a = static_cast<uint8_t>((x & 0xf000) << 4);
	return Color(r, g, b, a);
*/
	return ((x & 0x00F) << 4) | ((x & 0x00F0) << 8) | ((x & 0x0F00) << 12) | ((x & 0xF000) << 16);
}

uint32_t Color::from_RGB565(uint16_t x)
{
	return ((x & 0x001F) << 3) | ((x & 0x07E0) << 5) | ((x & 0xF800) << 8) | 0xFF000000;
}

uint8_t Color::to_G8(uint32_t rgba)
{
	uint8_t* c = reinterpret_cast<uint8_t*>(&rgba);
	return (c[0] + c[1] + c[2] + 1) / 3;
}

uint16_t Color::to_GA88(uint32_t rgba)
{
	uint8_t* c = reinterpret_cast<uint8_t*>(&rgba);
	uint8_t gray = static_cast<uint8_t>((c[0] + c[1] + c[2] + 1) / 3);
	return gray | (c[3] << 8);
}

uint16_t Color::to_RGBA5551(uint32_t rgba)
{
//	return static_cast<uint16_t>((r>>3) | (g>>3<<5) | (b>>3<<10) | (a>>7<<15));
	return static_cast<uint16_t>( // 0xAABBGGRR
		(rgba & 0x000000F8) >> 3 |
		(rgba & 0x0000F800) >> 6 |
		(rgba & 0x00F80000) >> 9 |
		(rgba & 0x80000000) >> 16);
}

uint16_t Color::to_RGBA4444(uint32_t rgba)
{
	return static_cast<uint16_t>(
		(rgba & 0x000000F0) >> 4 |
		(rgba & 0x0000F000) >> 12|
		(rgba & 0x00F00000) >> 20|
		(rgba & 0xF0000000) >> 28);
}

uint16_t Color::to_RGB565(uint32_t rgba)
{
	return static_cast<uint16_t>(
		(rgba & 0x0000F8) >> 3|
		(rgba & 0x00FC00) >> 5|
		(rgba & 0xF80000) >> 8);
}

bool Color::parse(const char* hex, uint32_t& rgba)
{
	// #RRGGBBAA
	if(!hex || hex[0] != '#' || std::strlen(hex) != 7)
		return false;

	if(sscanf(hex + 1, "%x", &rgba) != 1)
		return false;

	return true;
}

uint32_t ColorF::getColor() const
{
	constexpr float s = 255.999;
	uint8_t r = static_cast<uint8_t>(color.r * s);
	uint8_t g = static_cast<uint8_t>(color.g * s);
	uint8_t b = static_cast<uint8_t>(color.b * s);
	uint8_t a = static_cast<uint8_t>(color.a * s);
	return Color::rgba(r, g, b, a);
}

void ColorF::clamp()
{
	for(uint8_t c = 0; c < 4; ++c)
		color[c] = pea::clamp<float>(color[c], 0, 1);
}

ColorF ColorF::alphaBlend(const ColorF& foreground, const ColorF& background)
{
	const float &fa = foreground.color.a, &ba = (1 - fa) * background.color.a;
	float alpha = fa + ba;
	
	float rgb[3];
	for(uint8_t c = 0; c < 3; ++c)
		rgb[c] = (fa * foreground.color[c] + ba * background.color[c]) / alpha;
	return ColorF(rgb[0], rgb[1], rgb[2], alpha);
}

void ColorHSL::fromRGB(const vec3f& color)
{
	// find maxium and minimum value from red/green/blue components
	float min = color.r, max = color.g;
	if(min > max)
		std::swap(min, max);
	if(color.b < min)
		min = color.b;
	else if(color.b > max)
		max = color.b;

	float sum = min + max;
	luminance = sum/2;  // [0, 100]
	if(fuzzyEqual(min, max) || isZero(max))
	{
//		luminance = min * 100;
		hue = 0.0f;
		saturation = 0.0f;
		return;
	}

	float delta = max - min;
	float denorm = (luminance <= 0.5) ? sum:(2-sum);
	saturation =  delta/denorm;

	if(max == color.r)
		hue = (color.g - color.b)/delta;
	else if(max == color.g)
		hue = 2 + (color.b - color.r)/delta;
	else // (max == color.b)
		hue = 4 + (color.r - color.g)/delta;

	if(hue < 0)
		hue += 6.0;
	hue *= 60.0;
}

vec3f ColorHSL::toRGB() const
{
	assert(0 <= hue && hue < 360.0);
	assert(0 <= saturation && saturation <= 1.0);
	assert(0 <= luminance && luminance <= 1.0);

	float h = hue, s = saturation, l = luminance;
	h /= 60.0;  // range d[0, 6)

	int i = static_cast<int>(std::floor(h));
	float f = h - i;
	float p = l * (1 - s);
	float q = l * (1 - s * f);
	float t = l * (1 - s * (1-f));

	switch(i)
	{
	case 0: return vec3f(l, t, p); break;
	case 1: return vec3f(q, l, p); break;
	case 2: return vec3f(p, l, t); break;
	case 3: return vec3f(p, q, l); break;
	case 4: return vec3f(t, p, l); break;
	case 5: return vec3f(l, p, l); break;  // FIXME book printf("Error!");
	default: assert(false); return vec3f(0.0F, 0.0F, 0.0F); break;
	}
}
