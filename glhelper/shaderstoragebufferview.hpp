#pragma once

#include "gl.hpp"
#include "buffer.hpp"

#include <memory>

namespace gl
{
	class Buffer;

	/// Easy shader storage buffer handling.
	class ShaderStorageBufferView
	{
	public:
		/// Provides a shader storage view for a given buffer.
		/// \param [in] _name
		///		Name used for binding via shader - see ShaderObject::BindSSBO.
		ShaderStorageBufferView(std::shared_ptr<gl::Buffer> _buffer, const std::string& _name);

		~ShaderStorageBufferView();

		/// Returns buffer name (see constructor).
		const std::string& GetBufferName() const { return m_name; }

		/// Binds buffer if not already bound.
		void BindBuffer(GLuint _locationIndex) const { m_buffer->BindShaderStorageBuffer(_locationIndex); }

		/// Get the internal bound buffer resource.
		const std::shared_ptr<Buffer>& GetBuffer() const   { return m_buffer; }

	private:
		std::shared_ptr<Buffer> m_buffer;
		std::string m_name;
	};
}