#pragma once

#include "gl.hpp"

#include <memory>

namespace gl
{
	class Buffer;

	/// Large buffers with cached index access for the GP.
	/// OpenGL name: shader storage buffer object (SSBO)
	class ShaderStorageBufferView
	{
	public:
		/// Create the buffer
		ShaderStorageBufferView();
		~ShaderStorageBufferView();

		/// Bind the buffer data to this structured TextureBuffer.
		/// \param [in] _name Name used in shader programs if explicit binding
		///     is not used.
		Result Init(std::shared_ptr<gl::Buffer> _buffer, const std::string& _name);

		const std::string& GetBufferName() const { return m_name; }

		/// Binds buffer if not already bound.
		///
		/// Performs an Unmap if the buffer is currently maped.
		/// \attention Deleting a bound resource may cause unexpected errors. 
		///		The user is responsible for manual unbinding it (or overwriting the previous binding) before deleting.
		void BindBuffer(GLuint _locationIndex);

		/// Resets the binding of the given location to zero.
		static void ResetBinding(GLuint _locationIndex);

		/// Get the internal bound buffer resource.
		std::shared_ptr<Buffer> GetBuffer() const   { return m_buffer; }

	private:
		std::shared_ptr<Buffer> m_buffer;
		std::string m_name;

		
		/// Arbitrary value, based on observation: http://delphigl.de/glcapsviewer/gl_stats_caps_single.php?listreportsbycap=GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS
		static const unsigned int s_numSSBOBindings = 16;
		/// Currently bound SSBOs.
		static BufferId s_boundSSBOs[s_numSSBOBindings];
	};
}