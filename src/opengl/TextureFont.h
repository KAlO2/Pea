#ifndef PEA_OPENGL_TEXTURE_FONT_H_
#define PEA_OPENGL_TEXTURE_FONT_H_

#include <memory>

#include "graphics/Paint.h"
#include "graphics/Path.h"

namespace pea {

/*
	Glyph metrics:
	--------------

						  x_min                     x_max
						   |                         |
						   |<-------- width -------->|
				 ^         |                         |
				 |         +-------------------------+----------------- y_max
				 |         |    ggggggggg   ggggg    |     ^        ^
				 |         |   g:::::::::ggg::::g    |     |        |
				 |         |  g:::::::::::::::::g    |     |        |
				 |         | g::::::ggggg::::::gg    |     |        |
				 |         | g:::::g     g:::::g     |     |        |
	   offset_x -|-------->| g:::::g     g:::::g     |  offset_y    |
				 |         | g:::::g     g:::::g     |     |        |
				 |         | g::::::g    g:::::g     |     |        |
				 |         | g:::::::ggggg:::::g     |     |        |
				 |         |  g::::::::::::::::g     |     |      height
				 |         |   gg::::::::::::::g     |     |        |
	 baseline ---*---------|---- gggggggg::::::g-----*-------->     |
			   / |         |             g:::::g     |              |
		origin   |         | gggggg      g:::::g     |              |
				 |         | g:::::gg   gg:::::g     |              |
				 |         |  g::::::ggg:::::::g     |              |
				 |         |   gg:::::::::::::g      |              |
				 |         |     ggg::::::ggg        |              |
				 |         |        ggggggg          |              v
				 |         +-------------------------+----------------- y_min
				 |                                   |
				 |------------- advance_x ---------->|

*/
class TextureFont
{
private:
	class Impl;
	std::unique_ptr<Impl> impl;
	
public:
	/**
	 * @param[in] fontPath absolute path of a font, such as "/usr/share/fonts/truetype/freefont/FreeSans.ttf"
	 */
	TextureFont(const std::string& fontPath);
	~TextureFont();

	void setLineHeight(uint32_t height);
	uint32_t getLineHeight() const;

	void setWindowSize(int32_t width, int32_t height);

	/**
	 * @param[in] text
	 * @param[in] position  bottom left position.
	 * @param[in] color text color
	 * @param[in] scale
	 */
	void drawText(const std::string& text, const vec2f& position, const Paint& paint);
	
	void drawTextOnPath(const std::string& text, const Path& path, const Paint& paint);
};

}  // namespace pea
#endif  // PEA_OPENGL_TEXTURE_FONT_H_
