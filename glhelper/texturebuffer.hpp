#pragma once

#include "gl.hpp"

#include "textureformats.hpp"
#include "buffer.hpp"

#include <memory>

namespace gl
{
    /// Large buffers with cached index access for the GP.
	class TextureBufferView
	{
	public:
		TextureBufferView(const TextureBufferView&) = delete;
		void operator = (const TextureBufferView&) = delete;
		void operator = (TextureBufferView&&) = delete;

        /// Create the buffer
		TextureBufferView();
		TextureBufferView(TextureBufferView&& _moved);
		~TextureBufferView();
			
        /// Bind the buffer data to this TextureBuffer.
		Result Init(std::shared_ptr<Buffer> _buffer, TextureBufferFormat _format);
		Result Init(std::shared_ptr<Buffer> _buffer, TextureBufferFormat _format, GLintptr _offset, GLsizeiptr _numBytes);


		/// Binds buffer if not already bound.
		/// \attention Bindings are shared with texture!
		void BindBuffer(GLuint _locationIndex) const;

        /// Get the internal bound buffer resource.
        std::shared_ptr<Buffer> GetBuffer() const   { return m_buffer; }

	private:
		TextureId m_textureHandle;
        std::shared_ptr<Buffer> m_buffer;
	};
}