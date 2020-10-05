#include "opengl/Framebuffer.h"

#include <cassert>

#include "opengl/GL.h"
#include "util/Log.h"

using namespace pea;

static const char* TAG = "Framebuffer";

Framebuffer::Framebuffer(int32_t width, int32_t height):
		width(width),
		height(height),
		framebuffer(0),
		colorBuffer(0),
		depthStencilBuffer(0)
{
	assert(width > 0 && height > 0);
	glGenFramebuffers(1, &framebuffer);
}

Framebuffer::~Framebuffer()
{
	glDeleteFramebuffers(1, &framebuffer);
}

Texture Framebuffer::createTexture2D(int32_t internalFormat, int32_t format, int32_t type, uint32_t attachment)
{
	constexpr GLenum target = GL_TEXTURE_2D;
	Texture texture(target);
	texture.bind();

//	constexpr int32_t format = GL_RGBA; type = GL_UNSIGNED_BYTE
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	constexpr int32_t level = 0;
	glTexImage2D(target, level, internalFormat, width, height, 0, format, type, nullptr);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, target, texture.getName(), level);
	
	return texture;
}

Texture Framebuffer::createTextureRectangle(int32_t internalFormat, int32_t format, int32_t type, uint32_t attachment)
{
	constexpr GLenum target = GL_TEXTURE_RECTANGLE;
	Texture texture(target);
	texture.bind();

//	constexpr int32_t format = GL_RGBA; type = GL_UNSIGNED_BYTE
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	constexpr int32_t level = 0;
	glTexImage2D(target, level, internalFormat, width, height, 0, format, type, nullptr);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, target, texture.getName(), level);
	
	return texture;
}

uint32_t Framebuffer::createRenderBuffer(bool stencil)
{
	// create a renderbuffer object for depth and stencil attachment.
	uint32_t renderBuffer;
	glGenRenderbuffers(1, &renderBuffer);
	constexpr GLenum target = GL_RENDERBUFFER;
	glBindRenderbuffer(target, renderBuffer);
	GLenum internalformat, attachment;
	
	if(stencil)
	{
		internalformat = GL_DEPTH24_STENCIL8;
		attachment = GL_DEPTH_STENCIL_ATTACHMENT;
	}
	else
	{
		internalformat = GL_DEPTH_COMPONENT;
		attachment = GL_DEPTH_ATTACHMENT;
	}

	glRenderbufferStorage(target, internalformat, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderBuffer);
	
	return renderBuffer;
}

void Framebuffer::prepare()
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	
	// create a color attachment texture
//	colorBuffer = createTexture2D(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT0);
	
//	depthStencilBuffer = createRenderBuffer();

	// Always check wheter framebuffer is complete or not.
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
		slog.e(TAG, "Framebuffer is not complete! status=%4X", status);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}

void Framebuffer::unbind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::bindForReading() const
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
}

void Framebuffer::bindForWriting() const
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
}

void Framebuffer::blitFramebuffer(uint32_t mask, bool toDefault) const
{
	assert((mask & ~(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)) == 0);
	uint32_t src = toDefault? framebuffer: 0;
	uint32_t dst = toDefault? 0: framebuffer;
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src);
	GLenum filter = GL_NEAREST;  // GL_LINEAR
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, mask, filter);
}
/*
void Framebuffer::bindColorTexture() const
{
	glBindTexture(GL_TEXTURE_2D, colorBuffer);
}
*/
