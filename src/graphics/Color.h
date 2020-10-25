#ifndef PEA_GRAPHICS_COLOR_H_
#define PEA_GRAPHICS_COLOR_H_

#include <cassert>
#include <cstdint>
#include <string>

#include "math/vec3.h"
#include "math/vec4.h"

/*
#define COLOR_RGBA(r, g, b, a) \
	((uint32)((((r)&0xff)<<24)|(((g)&0xff)<<16)|(((b)&0xff)<<8)|((a)&0xff)))
#define COLOR_RGB(r, g, b) COLOR_RGBA(r, g, b, 0xff)
#define COLOR_RGBA_REV(r, g, b, a) COLOR_RGBA(a, b, g, r)
*/

namespace pea {


uint32_t color_cast(const vec4f& color);
vec4f    color_cast(uint32_t color);

vec3f hsv2rgb(const vec3f& hsv);

/**
 * a 32 bit RGBA color.
 * the red, green, blue, and alpha components are between 0 and 255
 */
class Color
{
public:
	enum Format: uint32_t
	{
		UNKNOWN = 0,

		// 8 bits
		G_8,  // gray

		// 16 bits
		GA_88,
		RGBA_5551,
		RGBA_4444,
		RGB_565,

		// 24 bits
		RGB_888,
		BGR_888,

		// 32 bits
		RGBA_8888,
		BGRA_8888,
		RGBA_1010102,
	};
/*
	union
	{
		struct { uint8_t r, g, b, a; };
		uint32_t rgba;
	};

	constexpr Color(): rgba(0) { }
	constexpr Color(uint32_t color): rgba(color) {}
	constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255): r(r), g(g), b(b), a(a) {}
*/
//	static uint32_t rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

	static uint32_t from_G8      (uint8_t  x);
	static uint32_t from_GA88    (uint16_t x);
	static uint32_t from_RGBX5551(uint16_t x);
	static uint32_t from_RGBA5551(uint16_t x);
	static uint32_t from_RGBA4444(uint16_t x);
	static uint32_t from_RGB565  (uint16_t x);
	static uint32_t from_RGB888  (uint8_t r, uint8_t g, uint8_t b);
	static uint32_t from_RGBA8888(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

	static uint8_t  to_G8(uint32_t rgba);
	static uint16_t to_GA88(uint32_t rgba);
	static uint16_t to_RGBA5551(uint32_t rgba);
	static uint16_t to_RGBA4444(uint32_t rgba);
	static uint16_t to_RGB565(uint32_t rgba);

	static uint8_t red(uint32_t color);
	static uint8_t green(uint32_t color);
	static uint8_t blue(uint32_t color);
	static uint8_t alpha(uint32_t color);
	
	/**
	 * regex (#)RRGGBB(AA) "#?([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})(?=[0-9a-fA-F]{2})"
	 * (#)RGB(A) "#?([0-9a-fA-F])([0-9a-fA-F])([0-9a-fA-F])(?=[0-9a-fA-F])"
	 */
	static bool parse(const char* hex, uint32_t& rgba);
	static std::string toString(const Color& color, bool alpha = true);

	/**
	 * @param[in] format color format
	 * @return sizeof the color format
	 */
	static constexpr uint32_t size(Format format);
	
	/**
	 * @param[in] format color format
	 * @return channel count
	 */
	static constexpr uint32_t channel(Format format);
#if 0
	int32_t luminance() const
	{
//		return (r*2 + g*5 + b)>>3;
		return (r*27 + g*92 + b*9)>>7;
	}
#endif
//	friend bool operator ==(const Color& lhs, const Color& rhs);

	static Color alphaBlend(const Color& foreground, const Color& background);
};

inline uint8_t Color::red(uint32_t color)   { return color;       }
inline uint8_t Color::green(uint32_t color) { return color >> 8;  }
inline uint8_t Color::blue(uint32_t color)  { return color >> 16; }
inline uint8_t Color::alpha(uint32_t color) { return color >> 24; }


constexpr uint32_t Color::size(Format format)
{
	switch(format)
	{
	case G_8:
		return 1;
	case GA_88:
	case RGBA_5551:
	case RGB_565:
		return 2;
	case RGB_888:
	case BGR_888:
		return 3;
	case RGBA_8888:
	case BGRA_8888:
		return 4;
	default:
		assert(false);
		return 0;
	}
}

constexpr uint32_t Color::channel(Format format)
{
	switch(format)
	{
	case G_8:
		return 1;
	case GA_88:
		return 2;
	case RGB_565:
	case RGB_888:
//	case BGR_565:
		return 3;
	case RGBA_5551:
	case RGBA_8888:
	case BGRA_8888:
		return 4;
	default:
		assert(false);
		return 0;
	}
}


/**
 * Class representing a color with four floats.
 * the red, green, blue, and alpha components are between 0.0 and 1.0
 */
class ColorF
{
private:
	vec4f color;
public:
	/**
	 * The default constructor
	 * creates an opaque, pure white (all components are 1.0F).
	 */
	constexpr ColorF():
			ColorF(1.0F, 1.0F, 1.0F, 1.0F)
	{}

	/**
	 * A constructor to specify component values right from the start.
	 * Note that values are not clamped, so use {@link #clamp} as you like.
	 *
	 * @param red The red component
	 * @param green The green component
	 * @param blue The blue component
	 * @param alpha The alpha component
	 */
	constexpr ColorF(float red, float green, float blue, float alpha = 1.0F):
			color(red, green, blue, alpha)
	{}
	
	constexpr ColorF(const vec4f& color):
			color(color)
	{}

	constexpr ColorF(const vec3f& color):
			ColorF(color.r, color.g, color.b)
	{}
//	Colorf(uint32_t color):
//		:Colorf(Color::fromR8G8B8A8(color))
//		r((color       & 0xff) / 255.0f),
//		g((color >>  8 & 0xff) / 255.0f),
//		b((color >> 16 & 0xff) / 255.0f),
//		a((color >> 24 & 0xff) / 255.0f)
//	{}

	constexpr ColorF(const uint32_t& c):
			ColorF((c & 255) / 255.0F, ((c >> 8)& 255) / 255.0F, ((c >> 16)& 255) / 255.0F, ((c >> 24)& 255) / 255.0F)
	{}

	static uint32_t getColor(const vec4f& color);
	
	uint32_t getColor() const;
	
	/**
	 * Clamp the components to the range 0.0 to 1.0
	 */
	void clamp();

	static ColorF alphaBlend(const ColorF& foreground, const ColorF& background);
};

/**
 * @brief Class representing an HSL format color.
 * The color values for hue, saturation, and luminance are stored in 32 bit floating point
 * variables. Hue is in range [0, 360), luminance and saturation are in [0.0, 1.0].
 * hue(red) = 0, hue(green) = 120, hue(blue) = 240.
 */
class ColorHSL
{
public:
	float hue, saturation, luminance;

public:
	ColorHSL() = default;
	ColorHSL(float hue, float saturation, float luminance):
		hue(hue), saturation(saturation), luminance(luminance)
	{}

	void fromRGB(const vec3f& color);
	vec3f toRGB() const;
};

}  // namespace pea
#endif  // PEA_GRAPHICS_COLOR_H_
