#pragma once

#include "gl.hpp"

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
		///
		/// Performs an Unmap if the buffer is currently maped.
		/// \attention Deleting a bound resource may cause unexpected errors. 
		///		The user is responsible for manual unbinding it (or overwriting the previous binding) before deleting.
		void BindBuffer(GLuint _locationIndex) const;

		/// Resets the binding of the given location to zero.
		static void ResetBinding(GLuint _locationIndex);

		/// Get the internal bound buffer resource.
		const std::shared_ptr<Buffer>& GetBuffer() const   { return m_buffer; }

	private:
		std::shared_ptr<Buffer> m_buffer;
		std::string m_name;

		
		/// Arbitrary value, based on observation: http://delphigl.de/glcapsviewer/gl_stats_caps_single.php?listreportsbycap=GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS
		static const unsigned int s_numSSBOBindings = 16;
		/// Currently bound SSBOs.
		static BufferId s_boundSSBOs[s_numSSBOBindings];
	};
}