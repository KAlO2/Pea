#ifndef PEA_OPENGL_FRAMEBUFFER_H_
#define PEA_OPENGL_FRAMEBUFFER_H_

#include <cstdint>

#include "opengl/Renderbuffer.h"
#include "opengl/Texture.h"

namespace pea {

class Framebuffer final
{
public:
/*
	enum Target: std::uint32_t
	{
		READ,
		WRITE,
		READ_WRITE,
	};
*/
private:
	int32_t width;
	int32_t height;
	
	uint32_t framebuffer;
	uint32_t colorBuffer;
	uint32_t depthStencilBuffer;
private:

	
public:
	Framebuffer(int32_t width, int32_t height);
	~Framebuffer();
	
	int32_t getWidth() const;
	int32_t getHeight() const;
	
	/**
	 * @param[in] attachment GL_COLOR_ATTACHMENT0
	 */
	Texture createTexture2D(int32_t internalFormat, int32_t format, int32_t type, uint32_t attachment);
	Texture createTextureRectangle(int32_t internalFormat, int32_t format, int32_t type, uint32_t attachment);
	
	Renderbuffer createRenderBuffer(bool enableStencil);
	
	void prepare();
	
	void bind() const;
	void unbind() const;
	
	void bindForReading() const;
	void bindForWriting() const;
	
	/**
	 * block transfer from/to default framebuffer
	 * @param[in] mask The bitwise OR of the flags indicating which buffers are to be copied. The 
	 *            allowed flags are GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT and 
	 *            GL_STENCIL_BUFFER_BIT.
	 * @param[in] toDefault true for copy to default framebuffer, otherwise copy default framebuffer
	 *            to this framebuffer.
	 */
	void blitFramebuffer(uint32_t mask, bool toDefault) const;
	
//	void bindColorTexture() const;
};

inline int32_t Framebuffer::getWidth()  const { return width;  }
inline int32_t Framebuffer::getHeight() const { return height; }

}  // namespace pea
#endif  // PEA_OPENGL_FRAMEBUFFER_H_
