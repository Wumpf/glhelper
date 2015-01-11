#include "texturebuffer.hpp"
#include "shaderobject.hpp"
#include "texture.hpp"

namespace gl
{
	TextureBufferView::TextureBufferView() :
		m_textureHandle(0)
	{
		GL_CALL(glCreateTextures, GL_TEXTURE_BUFFER, 1, &m_textureHandle);
	}

	TextureBufferView::~TextureBufferView()
	{
		// According to the specification it is not necessary to unbind the texture. All bindings reset themselves to zero.
		// http://docs.gl/gl4/glDeleteTextures
		// However this means, that glhelper's saved bindings are wrong!
		// Since they contain texture handles, this might result in rejected binding of new textures!

		// Iterating over all bindings is rather costly but reliable, easy and zero overhead for all other operations.
		for (unsigned int i = 0; i < Texture::s_numTextureBindings; ++i)
		{
			if (Texture::s_boundTextures[i] == m_textureHandle)
				Texture::s_boundTextures[i] = 0;
		}

        GL_CALL(glDeleteTextures, 1, &m_textureHandle);
	}

	Result TextureBufferView::Init(std::shared_ptr<Buffer> _buffer, TextureBufferFormat _format)
	{
		return Init(_buffer, _format, 0, _buffer->GetSize());
	}

	Result TextureBufferView::Init(std::shared_ptr<Buffer> _buffer, TextureBufferFormat _format, GLintptr _offset, GLsizeiptr _numBytes)
    {
		m_buffer = _buffer;
		GL_CALL(glTextureBufferRange, m_textureHandle, 
				static_cast<GLenum>(_format), _buffer->GetBufferId(), _offset, _numBytes);

        return Result::SUCCEEDED;
    }

	void TextureBufferView::BindBuffer(GLuint _locationIndex) const
	{
		Texture::Bind(m_textureHandle, _locationIndex);
	}
}