#include "opengl/TextureFont.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>

#include "opengl/Texture.h"
#include "opengl/Program.h"
#include "opengl/ShaderFactory.h"
#include "opengl/GL.h"
#include "scene/Camera.h"
#include "util/Log.h"


#define OUTPUT_TEXTURE_ATLAS 0
#if OUTPUT_TEXTURE_ATLAS
#	include <memory>
#	include "graphics/Image_PNG.h"
#endif

// ft2build.h is the 'entry point' for FreeType header file inclusion.
// all other FreeType header files should be accessed with macro names (after including ft2build.h).
#include <ft2build.h>
#include FT_FREETYPE_H

static const char* TAG = "TextureFont";

using namespace pea;

static constexpr uint8_t CHAR_BEGIN = 32;  // from space 32
static constexpr uint8_t CHAR_END   = 127; // to ~ 126

class TextureFont::Impl
{
private:
	FT_Library library;
	FT_Face face;

	std::string fontPath;
	uint32_t lineHeight;
	mat3f projection;
	
	uint32_t texture;
	uint32_t textureWidth;
	uint32_t textureHeight;
	
	struct Character
	{
		vec2u size;      // size of glyph
		vec2i bearing;   // offset from baseline to left/top of glyph
		vec2f advance;   // offset to advance to next glyph
		vec2i position;  // (left, bottom) position on the texture atlas
	};

	Character characters[CHAR_END - CHAR_BEGIN];

	uint32_t vao;
	uint32_t program;  // can be shared across TextureFont class
	
private:
	void drawVertexArray(const std::vector<vec4f>& vertices, const Paint& paint);
	
public:
	Impl(const std::string& fontPath) noexcept(false);
	~Impl();
	
	void setLineHeight(uint32_t height);
	uint32_t getLineHeight() const;

	void setWindowSize(int32_t width, int32_t height);
	
	void drawText(const std::string& text, const vec2f& position, const Paint& paint);
	
	void drawTextOnPath(const std::string& text, const Path& path, const Paint& paint);
};

TextureFont::Impl::Impl(const std::string& fontPath) noexcept(false):
		fontPath(fontPath),
		texture(0),
		textureWidth(0),
		textureHeight(0),
		vao(0),
		program(0)
{
	// Initialize the FreeType2 library
	FT_Error error = FT_Init_FreeType(&library);
	if(error != FT_Err_Ok)  // Error code 0 ( FT_Err_Ok) always means that the operation was successful
	{
		slog.e(TAG, "FT_Error (0x%02X): Could not initialize FreeType library", error);
		return;
	}

	// Load a font
	error = FT_New_Face(library, fontPath.c_str(), 0/* face_index */, &face);
	if(error != FT_Err_Ok)
	{
		std::ostringstream oss;
		oss << "FT_Error (" << error << "): Failed to load font " << fontPath;
		std::string message = oss.str();
		throw std::invalid_argument(message);
	}
	
	glGenVertexArrays(1, &vao);
	
	Program fontProgram(ShaderFactory::VERT_TEXTURE_FONT, ShaderFactory::FRAG_TEXTURE_FONT);
	assert(fontProgram.getUniformLocation("projection") == Shader::UNIFORM_MAT_PROJECTION);
	assert(fontProgram.getUniformLocation("texture0")   == Shader::UNIFORM_TEX_TEXTURE0);
	assert(fontProgram.getUniformLocation("textColor")  == Shader::UNIFORM_VEC_TEXT_COLOR);
	assert(fontProgram.getAttributeLocation("vertex")   == Shader::ATTRIBUTE_VEC_VERTEX);
	
	program = fontProgram.release();
}

TextureFont::Impl::~Impl()
{
	glDeleteProgram(program);
	glDeleteTextures(1, &texture);
	
	FT_Done_Face(face);
	FT_Done_FreeType(library);
}

static constexpr int32_t textureUnit = 0;

inline uint32_t TextureFont::Impl::getLineHeight() const
{
	return lineHeight;
}

void TextureFont::Impl::setLineHeight(uint32_t lineHeight)
{
	this->lineHeight = lineHeight;
	// a value of 0 for one of the dimensions means ‘same as the other’.
	FT_Set_Pixel_Sizes(face, 0, lineHeight);

	// characters have different sizes, choose the biggest one.
	uint32_t rowHeight = 0;
	constexpr uint32_t TEXTURE_WIDTH = 1024;  // texture width is fixed to power of two.
	int32_t x = 0, y = 0;  // left bottom point
	for(uint8_t ch = CHAR_BEGIN; ch < CHAR_END; ++ch)
	{
		assert(isprint(ch));

		// Load character glyph
		FT_Error error = FT_Load_Char(face, ch, FT_LOAD_RENDER);
		if(error != FT_Err_Ok)
		{
			slog.w(TAG, "FT_Error (%d): Failed to load glyph '%c'", error, ch);
			continue;
		}

		const FT_GlyphSlot& glyph = face->glyph;
		FT_Bitmap bitmap = glyph->bitmap;
		uint32_t charWidth = bitmap.width;
		uint32_t charHeight= bitmap.rows;
//		slog.i(TAG, "char %c height(%d) bitmap size(%d, %d) pitch=%d, advance.x=%d", ch, face->height, glyph->bitmap.width, glyph->bitmap.rows, glyph->bitmap.pitch, glyph->advance.x);
		/*
		glyph->advance.x's type is FT_Pos in <freetype2/ftimage.h>, which is a signed long int.
		The type FT_Pos is used to store vectorial coordinates. Depending on the context, these can
		represent distances in integer font units, or 16.16, or 26.6 fixed-point pixel coordinates.

		with GCC compiler on 64 bit platform, sizeof(long) == 8.
		apparently, 16.16 or 26.6 is 32 bits, so int32_t should be enough.

		sizeof(short int) <= sizeof(int) <= sizeof(long int) <= sizeof(long long int)
		Anyway, I hate so many int types there, short int | int | long int | long long int, will
		someday someone come up with a long long long int?
*/
//		static_assert(sizeof(int32_t) == sizeof(glyph->advance.x/* signed long */));
		Character character;
		character.size = vec2u(charWidth, charHeight);
		character.bearing = vec2i(glyph->bitmap_left, glyph->bitmap_top);
		// note that advance is number of 1/64 pixels
		character.advance = vec2f(glyph->advance.x / 64.0F, glyph->advance.y / 64.0F);
		
		if(x + charWidth < TEXTURE_WIDTH)
		{
			if(rowHeight < charHeight)
				rowHeight = charHeight;
		}
		else
		{
			x = 0;
			y += rowHeight;
			rowHeight = 0;  // start next line's height
		}
		character.position = vec2i(x, y);
		x += charWidth;
		
//		vec2i& position = character.position;
//		if(position.x < 0 || position.x >= textureWidth || position.y < 0 || position.y >= textureHeight)
//			slog.e(TAG, "ch %c position=(%d, %d)", ch, position.x, position.y);
		
		characters[ch - CHAR_BEGIN] = character;  // store character info for later usage
	}

	textureWidth = TEXTURE_WIDTH;
	textureHeight = y + rowHeight;
//	textureHeight = Texture::nextPowerOfTwo(textureHeight);

	// copy every character's pixel data to an image.
	std::unique_ptr<uint8_t> pixels(new uint8_t[textureWidth * textureHeight]);
	for(uint8_t ch = CHAR_BEGIN; ch < CHAR_END; ++ch)
	{
		FT_Error error = FT_Load_Char(face, ch, FT_LOAD_RENDER);
		if(error != FT_Err_Ok)
		{
			slog.w(TAG, "FT_Error (%d): Failed to load glyph '%c'", error, ch);
			continue;
		}
		
		FT_Bitmap bitmap = face->glyph->bitmap;
		const uint8_t* buffer = bitmap.buffer;
		const Character& character = characters[ch - CHAR_BEGIN];
		// character.size.width == glyph->bitmap.width <= glyph->bitmap.pitch
		// https://www.freetype.org/freetype2/docs/glyphs/glyphs-7.html
		// positive pitch: bitmap goes top down; negative pitch: bitmap goes bottom up.
		const int32_t& pitch  = bitmap.pitch;
		const uint32_t& width = bitmap.width;
		const uint32_t& height= bitmap.rows;  // or character.size.height;
		const vec2i& position = character.position;
		
		uint8_t* block = pixels.get() + position.y * textureWidth + position.x;
		for(uint32_t r = 0; r < height; ++r)
		{
			std::memcpy(block, buffer, width * sizeof(uint8_t));
			block  += textureWidth;
			buffer += pitch;
		}
	}

	// Create a texture that will be used to hold all ASCII glyphs
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	if(texture != 0)
		glDeleteTextures(1, &texture);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	
	// We require 1 byte alignment when uploading texture data
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	// GL_EXT_texture_rectangle & GL_ARB_texture_non_power_of_two, GL_OES_texture_npot,
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, textureWidth, textureHeight, 0, GL_RED, GL_UNSIGNED_BYTE, pixels.get());
	slog.d(TAG, "generated a %dx%d texture atlas", textureWidth, textureHeight);
	
	// Set texture options, Clamping to edges is important to prevent artifacts when scaling
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Linear filtering usually looks best for text
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
#if OUTPUT_TEXTURE_ATLAS
	Image_PNG image(textureWidth, textureHeight, Color::Format::G_8, pixels.release());
	std::string path = fontPath + ".png";
	image.save(path);
	slog.d(TAG, "texture image saved to %s", path.c_str());
#endif
}

void TextureFont::Impl::setWindowSize(int32_t width, int32_t height)
{
	assert(width > 0 && height > 0);
	projection = Camera::ortho(0.0F, width, 0.0F, height);
}

void TextureFont::Impl::drawVertexArray(const std::vector<vec4f>& vertices, const Paint& paint)
{
	assert(program != 0);
	glUseProgram(program);
	
	glBindVertexArray(vao);
	
	uint32_t vbo;
	glGenBuffers(1, &vbo);
	GL::bindVertexBuffer(vbo, Shader::ATTRIBUTE_VEC_VERTEX, vertices);
	
	uint32_t color = paint.getColor();
	vec4f textColor;
	textColor.r = Color::red(color)   / 255.0F;
	textColor.g = Color::green(color) / 255.0F;
	textColor.b = Color::blue(color)  / 255.0F;
	textColor.a = Color::alpha(color) / 255.0F;
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	Program::setUniform(Shader::UNIFORM_TEX_TEXTURE0, textureUnit);
	
	Program::setUniform(Shader::UNIFORM_MAT_PROJECTION, projection);
	Program::setUniform(Shader::UNIFORM_VEC_TEXT_COLOR, textColor);
	
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	
	glDeleteBuffers(1, &vbo);
	glBindVertexArray(0);
}

static std::string filterNonPrintableCharacter(const std::string& text)
{
	std::string s = text;
	// !std::isprint(ch)
	auto notPrintable = [](unsigned char ch) { return ch < CHAR_BEGIN || ch >= CHAR_END; };
	s.erase(std::remove_if(s.begin(), s.end(), notPrintable), s.end());
	return s;
}

void TextureFont::Impl::drawText(const std::string& text, const vec2f& position, const Paint& paint)
{
	std::string filteredText = filterNonPrintableCharacter(text);  // \r\n\t will be ignored here.
	const size_t length = filteredText.length();
	if(length == 0)
		return;
	
	// to make texture coordinates scale to [0.0 1.0]
	float textureScaleX = 1.0F / textureWidth;
	float textureScaleY = 1.0F / textureHeight;
	
	std::vector<vec4f> vertices(6 * length);  // vec4 = vec2 position + vec2 texcoord
	float offset = 0;
	float scale = paint.getTextScaleX();
//	static bool print = false;
	for(size_t i = 0; i < length; ++i)
	{
		const char& ch = filteredText[i];
		assert(std::isprint(ch));
		const Character& character = characters[ch - CHAR_BEGIN];
		
		float width = character.size.width;
		float height= character.size.height;
//		slog.d(TAG, "offset=%f, character.bearing.x=%d, scale=%f", offset, character.bearing.x, scale);

		float x0 = position.x + (offset + character.bearing.x) * scale;
		offset += character.advance.x;
		if((character.size.width || character.size.height) == 0)  // no need to draw space
			continue;
		
		float x1 = x0 + width * scale;
		float y1 = position.y + character.bearing.y * scale;
		float y0 = y1 - height * scale;
		//  ^ y                ^  t
		//  |  y1              |  t1
		//  |  y0              |  t0
		//  |  /x0  x1         |  /s0  s1
		//  +-----------> x    +-----------> s
		float s0 = character.position.x * textureScaleX;
		float s1 = (character.position.x + width) * textureScaleX;
		float t1 = character.position.y * textureScaleY;
		float t0 = (character.position.y + height) * textureScaleY;
		
		size_t j = 6 * i;
		vertices[j + 0] = vec4f(x0, y1, s0, t1);
		vertices[j + 1] = vec4f(x0, y0, s0, t0);
		vertices[j + 2] = vec4f(x1, y1, s1, t1);
		
		vertices[j + 3] = vec4f(x1, y1, s1, t1);
		vertices[j + 4] = vec4f(x0, y0, s0, t0);
		vertices[j + 5] = vec4f(x1, y0, s1, t0);
		
//		if(!print)
//		std::cout << vec2f(x0, y0) << " " << vec2f(x1, y1) << "   " << projection * vec3f(x0, y0, 1.0F) << ' ' << projection * vec3f(x1, y1, 1.0F) << '\n';

	}
//	print = true;
	drawVertexArray(vertices, paint);
}

void TextureFont::Impl::drawTextOnPath(const std::string& text, const Path& path, const Paint& paint)
{
	std::string filteredText = filterNonPrintableCharacter(text);  // \r\n\t will be ignored here.
	const size_t length = filteredText.length();
	if(length == 0)
		return;
	
	std::vector<float> intervals(length);
	float scale = paint.getTextScaleX();
	for(size_t i = 0; i < length; ++i)
	{
		const char& ch = filteredText[i];
		assert(std::isprint(ch));
	
		const Character& character = characters[ch - CHAR_BEGIN];
		intervals[i] = character.advance.x;  // * scale;
	}
	
	std::vector<vec4f> transforms(length);  // position and rotation, rigid transformation
	float offset = 0;
	path.lineSpace(intervals.data(), transforms.data(), length, offset);
	

	// to make texture coordinates scale to [0.0 1.0]
	float textureScaleX = 1.0F / textureWidth;
	float textureScaleY = 1.0F / textureHeight;
	
	std::vector<vec4f> vertices(6 * length);  // position and texcoord
	offset = 0;
	for(size_t i = 0; i < length; ++i)
	{
		const char& ch = text[i];
		const Character& character = characters[ch - CHAR_BEGIN];
		
		float width = character.size.width;
		float height= character.size.height;
//		slog.d(TAG, "offset=%f, character.bearing.x=%d, scale=%f", offset, character.bearing.x, scale);

		const vec4f& transform = transforms[i];
		const float& x     = transform.x;
		const float& y     = transform.y;
		const float& cos_a = transform.z;
		const float& sin_a = transform.w;
		
		float x0 = x + (offset + character.bearing.x) * scale;
		offset += character.advance.x;
		if((character.size.width || character.size.height) == 0)  // no need to draw space
			continue;
		
//		float x1 = x0 + width * scale;
		float y0 = y + (character.bearing.y - height) * scale;
//		float y0 = y1 - height * scale;
		float dx = width * scale;
		float dy = height * scale;
		
		// rotate at fixed point (x0, y0) with rotation (cos_a, sin_a).
		// those are relatieve to bl, namely (x, y);
		vec2f br(dx * cos_a, dx * sin_a);  // (dx, 0) * (cos_a, sin_a)
		vec2f tl(dy *-sin_a, dy * cos_a);  // (0, dy) * (cos_a, sin_a)
		vec2f tr(br.x + tl.x, br.y + tl.y);
		
		// relative to absolute
		vec2f bl(x0, y0);
		br += bl;
		tl += bl;
		tr += bl;
		static bool print = false;
		if(!print)
		{
			if(i == 0)
				std::cout << transforms[0] << "\n"
				<< "offset=" << offset << ", bearing.x=" << character.bearing.x << ", scale=" << scale << '\n'
				<< tl << "  " << tr << '\n'
						<< bl << "  " << br << '\n';
			print = true;
		}
		//  ^ y                ^  t
		//  |  y1              |  t1
		//  |  y0              |  t0
		//  |  /x0  x1         |  /s0  s1
		//  +-----------> x    +-----------> s
		float s0 = character.position.x * textureScaleX;
		float s1 = (character.position.x + width) * textureScaleX;
		float t1 = character.position.y * textureScaleY;
		float t0 = (character.position.y + height) * textureScaleY;
		
		size_t j = 6 * i;
		vertices[j + 0] = vec4f(tl.x, tl.y, s0, t1);
		vertices[j + 1] = vec4f(bl.x, bl.y, s0, t0);
		vertices[j + 2] = vec4f(tr.x, tr.y, s1, t1);
		
		vertices[j + 3] = vec4f(tr.x, tr.y, s1, t1);
		vertices[j + 4] = vec4f(bl.x, bl.y, s0, t0);
		vertices[j + 5] = vec4f(br.x, br.y, s1, t0);
		
//		if(!print)
//		std::cout << vec2f(x0, y0) << " " << vec2f(x1, y1) << "   " << projection * vec3f(x0, y0, 1.0F) << ' ' << projection * vec3f(x1, y1, 1.0F) << '\n';

	}
	
	drawVertexArray(vertices, paint);
}

TextureFont::TextureFont(const std::string& fontPath):
		impl(std::make_unique<TextureFont::Impl>(fontPath))
{

}

TextureFont::~TextureFont() = default;

void TextureFont::setLineHeight(uint32_t height)
{
	impl->setLineHeight(height);
}

uint32_t TextureFont::getLineHeight() const
{
	return impl->getLineHeight();
}

void TextureFont::setWindowSize(int32_t width, int32_t height)
{
	impl->setWindowSize(width, height);
}

void TextureFont::drawText(const std::string& text, const vec2f& position, const Paint& paint)
{
	impl->drawText(text, position, paint);
}

void TextureFont::drawTextOnPath(const std::string& text, const Path& path, const Paint& paint)
{
	impl->drawTextOnPath(text, path, paint);
}
