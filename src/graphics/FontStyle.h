#ifndef PEA_GRAPHICS_FONT_STYLE_H_
#define PEA_GRAPHICS_FONT_STYLE_H_

namespace pea {

/**
 * A font style object.
 *
 * This class represents a single font style which is a pair of weight value and slant value.
 * Here are common font styles examples:
 * <p>
 * <pre>
 * <code>
 * final FontStyle NORMAL = new FontStyle(FONT_WEIGHT_NORMAL, FONT_SLANT_UPRIGHT);
 * final FontStyle BOLD = new FontStyle(FONT_WEIGHT_BOLD, FONT_SLANT_UPRIGHT);
 * final FontStyle ITALIC = new FontStyle(FONT_WEIGHT_NORMAL, FONT_SLANT_ITALIC);
 * final FontStyle BOLD_ITALIC = new FontStyle(FONT_WEIGHT_BOLD, FONT_SLANT_ITALIC);
 * </code>
 * </pre>
 * </p>
 *
 */
class FontStyle
{
public:
	enum class Weight: int16_t
	{
		INVISIBLE   =   0,
		THIN        = 100,
		EXTRA_LIGHT = 200,
		LIGHT       = 300,
		NORMAL      = 400,
		MEDIUM      = 500,
		SEMI_BOLD   = 600,
		BOLD        = 700,
		EXTRA_BOLD  = 800,
		BLACK       = 900,
		EXTRA_BLACK =1000,
	};
	
	enum class Width: int8_t
	{
		ULTRA_CONDENSED = 1,
		EXTRA_CONDENSED = 2,
		      CONDENSED = 3,
		 SEMI_CONSENSED = 4,
		
		         NORMAL = 5,
		
		  SEMI_EXPANDED = 6,
		       EXPANDED = 7,
		 EXTRA_EXPANDED = 8,
		 ULTRA_EXPANDED = 9,
	};

	enum class Slant: int8_t
	{
		UPRIGHT,
		ITALIC,
		OBLIQUE,
	};

	// frequently-used font style
	static constexpr FontStyle NORMAL = FontStyle(Weight::NORMAL, Width::NORMAL, Slant::UPRIGHT);
	static constexpr FontStyle BOLD   = FontStyle(Weight::BOLD,   Width::NORMAL, Slant::UPRIGHT);
	static constexpr FontStyle ITALIC = FontStyle(Weight::NORMAL, Width::NORMAL, Slant::ITALIC);
	static constexpr FontStyle BOLD_ITALIC = FontStyle(Weight::BOLD, Width::NORMAL, Slant::ITALIC);
	
private:
	uint32_t value;
	
public:
	constexpr FontStyle():
			FontStyle(Weight::NORMAL, Width::NORMAL, Slant::UPRIGHT)
	{
	}
	
	constexpr FontStyle(int weight, Width width, Slant slant):
			value((weight & 0xFFFF) | static_cast<uint32_t>(width) << 16 | static_cast<uint32_t>(slant) << 24)
	{
		assert(Weight::INVISIBLE < weight && weight <= Weight::EXTRA_BLACK);
		assert(Width::ULTRA_CONDENSED < width && width < Width::ULTRA_EXPANDED);
		assert(Slant::UPRIGHT <= slant && slant <= Slant::OBLIQUE);
	}
	
	friend bool operator ==(const FontStyle& lhs, const FontStyle& rhs)
	{
		return lhs.value == rhs.value;
	}
	
	int   getWeight() const { return value & 0xFFFF;       }
	Width getWidth()  const { return (value >> 16) & 0xFF; }
	Slant getSlant()  const { return (value >> 24) & 0xFF; }
	
};

}  // namespace pea
#endif  // PEA_GRAPHICS_FONT_STYLE_H_
