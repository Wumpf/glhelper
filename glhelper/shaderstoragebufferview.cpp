#include "shaderstoragebuffer.hpp"
#include "shaderobject.hpp"
#include "buffer.hpp"
#include "utils/flagoperators.hpp"

namespace gl
{
	BufferId ShaderStorageBufferView::s_boundSSBOs[ShaderStorageBufferView::s_numSSBOBindings];

	ShaderStorageBufferView::ShaderStorageBufferView()
	{
        // No additional resources required
	}

	ShaderStorageBufferView::~ShaderStorageBufferView()
	{
		// According to the specification it is not necessary to unbind the buffer. All bindings reset themselves to zero.
		// http://docs.gl/gl4/glDeleteBuffers (performed by gl::Buffer)
		// However this means, that glhelper's saved bindings are wrong.
		// Iterating over all bindings is rather costly but reliable, easy and zero overhead for all other operations.
		for (unsigned int i = 0; i < s_numSSBOBindings; ++i)
		{
			if (s_boundSSBOs[i] == m_buffer->GetBufferId())
				s_boundSSBOs[i] = 0;
		}
	}

    Result ShaderStorageBufferView::Init(std::shared_ptr<gl::Buffer> _buffer, const std::string& _name)
    {
        m_buffer = _buffer;
		m_name = _name;

        return gl::Result::SUCCEEDED;
    }

	void ShaderStorageBufferView::BindBuffer(GLuint _locationIndex)
	{
		GLHELPER_ASSERT(_locationIndex < s_numSSBOBindings,
			"Can't bind shader object buffer to slot " << _locationIndex << ". Maximum number of slots is " << s_numSSBOBindings);

		if (m_buffer->m_mappedData != nullptr && static_cast<GLenum>(m_buffer->m_usageFlags & Buffer::Usage::MAP_PERSISTENT) == 0)
			m_buffer->Unmap();

		if (s_boundSSBOs[_locationIndex] != m_buffer->GetBufferId())
		{
            GL_CALL(glBindBufferBase, GL_SHADER_STORAGE_BUFFER, _locationIndex, m_buffer->GetBufferId());
			s_boundSSBOs[_locationIndex] = m_buffer->GetBufferId();
		}
	}

	void ShaderStorageBufferView::ResetBinding(GLuint _locationIndex)
	{
		GLHELPER_ASSERT(_locationIndex < s_numSSBOBindings,
			"Can't bind shader object buffer to slot " << _locationIndex << ". Maximum number of slots is " << s_numSSBOBindings);
		
		GL_CALL(glBindBufferBase, GL_SHADER_STORAGE_BUFFER, _locationIndex, 0);
		s_boundSSBOs[_locationIndex] = 0;
	}
}