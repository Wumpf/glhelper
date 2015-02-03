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

		/// Creates a texture buffer view that provides access to an entire buffer.
		TextureBufferView(const std::shared_ptr<Buffer>& _buffer, TextureBufferFormat _format);
		/// Creates a texture buffer view that provides access to a part of a buffer.
		TextureBufferView(const std::shared_ptr<Buffer>& _buffer, TextureBufferFormat _format, GLintptr _offset, GLsizeiptr _numBytes);

		TextureBufferView(TextureBufferView&& _moved);
		~TextureBufferView();


		/// Binds buffer if not already bound.
		/// \attention Bindings are shared with texture!
		void BindBuffer(GLuint _locationIndex) const;

        /// Get the internal bound buffer resource.
        const std::shared_ptr<Buffer>& GetBuffer() const   { return m_buffer; }

	private:
		TextureId m_textureHandle;
        std::shared_ptr<Buffer> m_buffer;
	};
}