#ifndef PEA_OPENGL_RENDERBUFFER_H_
#define PEA_OPENGL_RENDERBUFFER_H_

#include <cstdint>

namespace pea {

class Renderbuffer
{
private:
	uint32_t buffer;

public:
	Renderbuffer();
	~Renderbuffer();
	
	Renderbuffer(const Renderbuffer& other) = delete;
	Renderbuffer& operator =(const Renderbuffer& other) = delete;
	
	Renderbuffer(Renderbuffer&& other) noexcept;
	Renderbuffer& operator =(Renderbuffer&& other) noexcept;
	
	/**
	 * @return texture name.
	 */
	const uint32_t& getName() const;
	
	void bind() const;
};

inline const uint32_t& Renderbuffer::getName() const { return buffer; }

}  // namespace pea
#endif  // PEA_OPENGL_RENDERBUFFER_H_
