#ifndef PEA_GRAPHICS_CANVAS_H_
#define PEA_GRAPHICS_CANVAS_H_

#include <memory>
#include <string>

#include "math/mat3.h"
#include "math/vec4.h"
#include "graphics/Rect.h"

namespace pea {

class Image;
class Paint;

/**
 * The Canvas class holds the "draw" calls. To draw something, you need 4 basic components: A Bitmap
 * to hold the pixels, a Canvas to host the draw calls (writing into the bitmap), a drawing
 * primitive (e.g. Rect, Path, text, Bitmap), and a paint (to describe the colors and styles for the
 * drawing).
 */
class Canvas
{
private:
	static constexpr int32_t MAXMIMUM_BITMAP_SIZE = 32766;  // 2^15 = 32768

	std::unique_ptr<Image> bitmap;
	mat3f transform;
	
public:
	Canvas(std::unique_ptr<Image> bitmap);
	
	vec2i getSize() const;
	
	/**
	 * Preconcat the current matrix with the specified translation
	 *
	 * @param dx The distance to translate in X
	 * @param dy The distance to translate in Y
	*/
	void translate(float dx, float dy);
	
	/**
	 * Preconcat the current matrix with the specified rotation.
	 *
	 * @param angle The amount to rotate, in radians
	 */
	void rotate(float angle);
	
	/**
	 * Preconcat the current matrix with the specified rotation.
	 *
	 * @param degrees The amount to rotate, in degrees
	 * @param px The x-coord for the pivot point (unchanged by the rotation)
	 * @param py The y-coord for the pivot point (unchanged by the rotation)
	 */
	void rotate(float angle, float px, float py);
	
	/**
	 * Preconcat the current matrix with the specified scale.
	 *
	 * @param sx The amount to scale in X
	 * @param sy The amount to scale in Y
	 */
	void scale(float sx, float sy);
	
	/**
	 * Preconcat the current matrix with the specified scale.
	 *
	 * @param sx The amount to scale in X
	 * @param sy The amount to scale in Y
	 * @param px The x-coord for the pivot point (unchanged by the scale)
	 * @param py The y-coord for the pivot point (unchanged by the scale)
	 */
	void scale(float sx, float sy, float px, float py);
	
	/**
	 * Preconcat the current matrix with the specified skew.
	 *
	 * @param sx The angle to skew in X
	 * @param sy The angle to skew in Y
	 */
	void skew(float sx, float sy);
	
	/**
	 * Completely replace the current matrix with the specified matrix.
	 *
	 * <strong>Note:</strong> it is recommended to use {@link #concat(Matrix)},
	 * {@link #scale(float, float)}, {@link #translate(float, float)} and {@link #rotate(float)}
	 * instead of this method.
	 *
	 * @param matrix The matrix to replace the current matrix with.
	 *
	 * @see #concat(const mat3f& transform)
	 */
	void setTransform(const mat3f& transform);
	
	/**
	 * @return the current transformation matrix.
	 */
	const mat3f& getTransform() const;
	
	// draw methods
	void clipRect(const Rect<int32_t>& region);
	

	/**
	 * Fill the entire canvas' bitmap (restricted to the current clip) with the specified color,
	 * using srcover porterduff mode.
	 *
	 * @param color the color to draw onto the canvas
	 */
	void drawColor(const vec4f& color);
	
	void drawPoint(const vec2f& point, const Paint& paint);
	void drawPoint(const vec2f* point, size_t count, const Paint& paint);
	
	void drawLine(const vec2f& p0, const vec2f& p1, const Paint& paint);
	
	void drawRect(const Rect<int32_t>& rect, const Paint& paint);
	void drawRect(const Rect<float>& rect, const Paint& paint);
	
	void drawRoundRect(const Rect<float>& rect, float radius, const Paint& paint);
	
	/**
	 * Draw the specified round-rect using the specified paint. The roundrect will be filled or
	 * framed based on the Style in the paint.
	 *
	 * @param rect The rectangular bounds of the roundRect to be drawn
	 * @param rx The x-radius of the oval used to round the corners
	 * @param ry The y-radius of the oval used to round the corners
	 * @param paint The paint used to draw the roundRect
	 */
	void drawRoundRect(const Rect<float>& rect, float rx, float ry, const Paint& paint);
	
	void drawCircle(float x, float y, float radius, const Paint& paint);
	
	void drawText(const char* text, size_t count, const vec2f& position, const Paint& paint);
	void drawText(const std::string& text, const vec2f& position, const Paint& paint);
	
};

inline void Canvas::drawText(const std::string& text, const vec2f& position, const Paint& paint)
{
	drawText(text.data(), text.size(), position, paint);
}

}  // namespace pea
#endif  // PEA_GRAPHICS_CANVAS_H_
