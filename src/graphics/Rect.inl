#include <cstdint>
//#include <type_traits>
#include <utility>

#ifndef PEA_GRAPHICS_RECT_H_
#error "please #include \"graphics/Rect.h\""
#endif

namespace pea {

template <typename T>
Rect<T>::Rect():
		Rect(0, 0, 0, 0)
{
}

template <typename T>
Rect<T>::Rect(T left, T right, T top, T bottom):
		left(left),
		right(right),
		top(top),
		bottom(bottom)
{
}

template <typename T>
Rect<T>& Rect<T>::operator = (const Rect<T>& other)
{
	if(this != &other)
	{
		this->left   = other.left;
		this->right  = other.right;
		this->top    = other.top;
		this->bottom = other.bottom;
	}
	return *this;
}

template <typename T>
bool Rect<T>::isEmpty() const
{
	return left >= right || top >= bottom;
}

template <typename T>
void Rect<T>::setEmpty()
{
	left = right = top = bottom = 0;
}

template <typename T>
void Rect<T>::sort()
{
	if(left > right)
		std::swap(left, right);
	if(top > bottom)
		std::swap(top, bottom);
}

template <typename T>
T Rect<T>::getWidth() const
{
	return right - left;
}

template <typename T>
T Rect<T>::getHeight() const
{
	return bottom - top;
}

template <typename T>
bool Rect<T>::contains(T x, T y) const
{
	if(isEmpty())  // check for empty first
		return false;
	
	return left <= x && x < right &&
			top <= y && y < bottom;
}

template <typename T>
bool Rect<T>::contains(const Rect<T>& rect) const
{
	if(isEmpty())  // check for empty first
		return false;
	
	return left <= rect.left && right <= rect.right &&
			top <= rect.top && bottom <= rect.bottom;
}

template <typename T>
void Rect<T>::union_(T x, T y)
{
	if(x < left)
		left = x;
	else if (x > right)
		right = x;
	
	if(y < top)
		top = y;
	else if (y > bottom)
		bottom = y;
}

template <typename T>
void Rect<T>::union_(const Rect<T>& rect)
{
	if(rect.isEmpty())
		return;
	
	if(isEmpty())
	{
		*this = rect;
		return;
	}
	
	if(rect.left < left)
		left = rect.left;
	if(rect.right > right)
		right = rect.right;
	if(rect.top < top)
		top = rect.top;
	if(rect.bottom > bottom)
		bottom = rect.bottom;
}

template <typename T>
void Rect<T>::offset(T dx, T dy)
{
	left += dx;  right  += dx;
	top  += dy;  bottom += dy;
}

template <typename T>
void Rect<T>::inset(T x)
{
	left   += x;
	top    += x;
	right  -= x;
	bottom -= x;
}

template <typename T>
void Rect<T>::scale(float factor)
{
	if(factor == 1.0F)
		return;
	
	if constexpr(std::is_floating_point<T>::value)
	{
		left  *= factor;
		right *= factor;
		top   *= factor;
		bottom*= factor;
	}
	else
	{
		auto round = [&factor](float x) { return static_cast<float>(x * factor + 0.5F); };
		left   = round(left);
		right  = round(right);
		top    = round(top);
		bottom = round(bottom);
	}
}

}  // namespace pea
