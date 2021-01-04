#ifndef PEA_GRAPHICS_PAINT_H_
#define PEA_GRAPHICS_PAINT_H_

#include <cinttypes>
#include <string>

#include "math/vec4.h"

namespace pea {

class Typeface;

/**
 * The Paint class holds the style and color information about how to draw geometries, text and
 * bitmaps.
 */
class Paint
{
public:
	enum class Alignment: std::uint8_t
	{
		LEFT,
		CNETER,
		RIGHT,
	};
	
	enum class Style: uint8_t
	{
		/**
		 * Geometry and text drawn with this style will be filled, ignoring all
		 * stroke-related settings in the paint.
		 */
		FILL = 0,
		
		/**
		 * Geometry and text drawn with this style will be stroked, respecting
		 * the stroke-related fields on the paint.
		 */
		STROKE = 1,
		
		/**
		 * Geometry and text drawn with this style will be both filled and
		 * stroked at the same time, respecting the stroke-related fields on
		 * the paint. This mode can give unexpected results if the geometry
		 * is oriented counter-clockwise. This restriction does not apply to
		 * either FILL or STROKE.
		 */
		FILL_AND_STROKE = 2,
	};
	
	/**
	 * Paint flag that enables antialiasing when drawing.
	 *
	 * <p>Enabling this flag will cause all draw operations that support antialiasing to use it.</p>
	 *
	 * @see #Paint(int)
	 * @see #setFlags(int)
	 */
	static constexpr uint32_t ANTI_ALIAS_FLAG = 0x01;

private:
	union
	{
		struct
		{
			uint32_t antiAlias: 1;
			uint32_t dither: 1;
			uint32_t capType: 2;
			uint32_t joinType: 2;
			Style    style: 2;
			uint32_t filterQuality: 2;
			uint32_t blendMode: 8; // only need 5-6?
			bool underlineText: 1;
			bool strikeThroughText: 1;
			bool overlineText: 1;
			
		} bitfields;
		uint32_t flags;
	};

	vec4f color;
	vec4f underlineColor;
	vec4f strikeThroughColor;
	vec4f overlineColor;
	
	float textSize;
	float textScaleX;
	float strokeWidth;
	Alignment alignment;
	Typeface* typeface;
	
public:
	Paint();
	
	/**
	 * Create a new paint with the specified flags. Use setFlags() to change
	 * these after the paint is created.
	 *
	 * @param[in] flags initial flag bits, as if they were passed via setFlags().
	 */
	explicit Paint(uint32_t flags);

	/**
	 * Set the paint's text size. This value must be > 0
	 *
	 * @param[in] textSize set the paint's text size in pixel units.
	 */
	void setTextSize(float size);
	
	/**
	 * Return the paint's text size.
	 *
	 * @return the paint's text size in pixel units.
	 */
	float getTextSize() const;
	
	void setTypeface(Typeface* typeface) { this->typeface = typeface; }
	Typeface* getTypeface() const        { return typeface;           }
	
	void setUnderlineColor(const vec4f& color)      { underlineColor     = color; }
	void setStrikeThroughColor(const vec4f& color)  { strikeThroughColor = color; }
	void setOverlineColor(const vec4f& color)       { overlineColor      = color; }
	
	const vec4f& getUnderlineColor() const     { return underlineColor;     }
	const vec4f& getStrikeThroughColor() const { return strikeThroughColor; }
	const vec4f& getOverlineColor() const      { return overlineColor;      }
	
	bool isUnderlineText() const     { return bitfields.underlineText;     }
	bool isStrikeThroughText() const { return bitfields.strikeThroughText; }
	bool isOverlineText() const      { return bitfields.overlineText;      }
	Style getStyle() const           { return bitfields.style;             }
	
	void setUnderlineText(bool underlineText)         { bitfields.underlineText     = underlineText;     }
	void setStrikeThroughText(bool strikeThroughText) { bitfields.strikeThroughText = strikeThroughText; }
	void setOverlineText(bool overlineText)           { bitfields.overlineText      = overlineText;      }
	void setStyle(Style style)                        { bitfields.style             = style;             }
	
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

	void setColor(const vec4f& color);
	void setStrokeWidth(float width);
	
	const vec4f& getColor() const;
	float getStrokeWidth() const;
	
	/**
	 * Return the width of the text.
	 *
	 * @param text  The text to measure. Cannot be null.
	 * @param start The index of the first character to start measuring
	 * @param end   1 beyond the index of the last character to measure
	 * @return      The width of the text
	 */
	float measureText(const char32_t* text, size_t count) const;
	float measureText(const std::u32string& text) const;
	
	float measureText(const char* text, size_t count) const;
	float measureText(const std::string& text) const;
};



inline void         Paint::setColor(const vec4f& color)    { this->color = color;}
inline const vec4f& Paint::getColor() const                { return color;       }

inline void Paint::setStrokeWidth(float width) { this->strokeWidth = width; }

inline float    Paint::getTextSize() const    { return textSize;    }
inline float    Paint::getStrokeWidth() const { return strokeWidth; }
inline float    Paint::getTextScaleX() const  { return textScaleX;  }

}  // namespace pea
#endif  // PEA_GRAPHICS_PAINT_H_
