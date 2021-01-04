#ifndef PEA_GRAPHICS_TYPEFACE_H_
#define PEA_GRAPHICS_TYPEFACE_H_

#include <cinttypes>
#include <string>

namespace pea {

class Typeface
{
public:
	enum Style: uint8_t
	{
		NORMAL = 0,
		BOLD   = 1,
		ITALIC = 2,
		BOLD_ITALIC = BOLD | ITALIC,
	};
	static constexpr uint8_t STYLE_COUNT = 4;
	
private:
	static constexpr uint32_t STYLE_MASK = 3;
	Style style;
	float weight;

public:
	static Typeface* create(const std::string& familyName, Typeface::Style style);
	
public:
	Typeface();
	virtual ~Typeface();
	
	Style getStyle() const;
	
	bool isBold() const;
	bool isItalic() const;
};

inline Typeface::Style Typeface::getStyle() const { return style; }
inline bool Typeface::isBold() const   { return (style & Typeface::BOLD  ) != 0; }
inline bool Typeface::isItalic() const { return (style & Typeface::ITALIC) != 0; }

}  // namespace pea
#endif  // PEA_GRAPHICS_TYPEFACE_H_
