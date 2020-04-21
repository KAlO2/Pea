#ifndef PEA_GRAPHICS_PAINT_H_
#define PEA_GRAPHICS_PAINT_H_

#include <cstdint>

namespace pea {

class Paint
{
public:
	enum class Alignment: std::uint8_t
	{
		LEFT,
		CNETER,
		RIGHT,
	};
private:
	uint32_t color;
	float textSize;
	float textScaleX;
	float strokeWidth;
	Alignment alignment;
	
public:
	Paint();
	
	/**
	 * Set the paint's text size. This value must be > 0
	 *
	 * @param textSize set the paint's text size in pixel units.
	 */
	void setTextSize(float size);
	
	/**
	 * Return the paint's text size.
	 *
	 * @return the paint's text size in pixel units.
	 */
	float getTextSize() const;
	
	/**
	 * Set the paint's horizontal scale factor for text. The default value is 1.0. Values > 1.0 will
	 * stretch the text wider. Values < 1.0 will stretch the text narrower.
	 *
	 * @param scaleX set the paint's scale in X for drawing/measuring text.
	 */
	void setTextScaleX(float scale);
	
	/**
	 * Return the paint's horizontal scale factor for text. The default value is 1.0.
	 *
	 * @return the paint's scale factor in X for drawing/measuring text
	 */
	float getTextScaleX() const;

	void setColor(uint32_t color);
	void setStrokeWidth(float width);
	
	uint32_t getColor() const;
	float    getStrokeWidth() const;
	
};



inline void Paint::setColor(uint32_t color)    { this->color = color;}
inline void Paint::setStrokeWidth(float width) { this->strokeWidth = width; }

inline float    Paint::getTextSize() const    { return textSize;    }
inline uint32_t Paint::getColor() const       { return color;       }
inline float    Paint::getStrokeWidth() const { return strokeWidth; }
inline float    Paint::getTextScaleX() const  { return textScaleX;  }

}  // namespace pea
#endif  // PEA_GRAPHICS_PAINT_H_
