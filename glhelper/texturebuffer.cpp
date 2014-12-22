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
        GL_CALL(glDeleteTextures, 1, &m_textureHandle);
	}

	Result TextureBufferView::Init(std::shared_ptr<Buffer> _buffer, TextureBufferFormat _format)
	{
		return Init(_buffer, _format, 0, _buffer->GetSize());
	}

	Result TextureBufferView::Init(std::shared_ptr<Buffer> _buffer, TextureBufferFormat _format, std::uint32_t _offset, std::uint32_t _numBytes)
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