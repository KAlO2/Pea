#ifndef PEA_GRAPHICS_CANVAS_H_
#define PEA_GRAPHICS_CANVAS_H_

#include <memory>

#include "math/vec2.h"


namespace pea {

class Image;
class Paint;

class Canvas
{
private:
	std::unique_ptr<Image> bitmap;
	
public:
	Canvas(std::unique_ptr<Image> bitmap);
	
	void drawLine(const vec2f& p0, const vec2f& p1, Paint* paint);

};

}  // namespace pea
#endif  // PEA_GRAPHICS_CANVAS_H_
