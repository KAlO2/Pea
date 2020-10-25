#include "opengl/Renderbuffer.h"

#include "opengl/GL.h"

using namespace pea;

Renderbuffer::Renderbuffer():
		buffer(0)
{
	glGenRenderbuffers(1, &buffer);
}

Renderbuffer::~Renderbuffer()
{
	glDeleteRenderbuffers(1, &buffer);
}

Renderbuffer::Renderbuffer(Renderbuffer&& other) noexcept:
		buffer(other.buffer)
{
	other.buffer = 0;
}

Renderbuffer& Renderbuffer::operator =(Renderbuffer&& other) noexcept
{
	buffer = other.buffer;
	other.buffer = 0;
	return *this;
}

void Renderbuffer::bind() const
{
	glBindRenderbuffer(GL_RENDERBUFFER, buffer);
}
