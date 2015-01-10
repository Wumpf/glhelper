#include "buffer.hpp"
#include "utils/flagoperators.hpp"

namespace gl
{
	Buffer::VertexBufferBinding Buffer::s_boundVertexBuffer[s_numVertexBindings];
	Buffer* Buffer::s_boundIndexBuffer = nullptr;

	Buffer::Buffer(GLsizeiptr _sizeInBytes, Usage _usageFlags, const void* _data) :
        m_sizeInBytes(_sizeInBytes),
		m_glMapAccess(0),
		m_usageFlags(_usageFlags),

		m_mappedDataOffset(0),
		m_mappedDataSize(0),
		m_mappedData(nullptr)
    {
		GLHELPER_ASSERT(static_cast<uint32_t>(m_usageFlags & Usage::EXPLICIT_FLUSH) == 0 || static_cast<uint32_t>(m_usageFlags & Usage::MAP_PERSISTENT) > 0,
			   "EXPLICIT_FLUSH only valid in combination with PERSISTENT");
		GLHELPER_ASSERT(static_cast<uint32_t>(m_usageFlags & Usage::MAP_COHERENT) == 0 || static_cast<uint32_t>(m_usageFlags & Usage::MAP_PERSISTENT) > 0,
			   "MAP_COHERENT only valid in combination with PERSISTENT");

		GL_CALL(glCreateBuffers, 1, &m_bufferObject);
		GL_CALL(glNamedBufferStorage, m_bufferObject, _sizeInBytes, _data, static_cast<GLbitfield>(_usageFlags));

		if (any(m_usageFlags & Usage::MAP_READ))
			m_glMapAccess = static_cast<uint32_t>(m_usageFlags & Usage::MAP_WRITE) > 0 ? GL_READ_WRITE : GL_READ_ONLY;
        else
			m_glMapAccess = static_cast<uint32_t>(m_usageFlags & Usage::MAP_WRITE) > 0 ? GL_WRITE_ONLY : 0;


		if (any(m_usageFlags & Usage::MAP_PERSISTENT))
		{
			m_glMapAccess |= GL_MAP_PERSISTENT_BIT;
			if (static_cast<uint32_t>(m_usageFlags & Usage::EXPLICIT_FLUSH) > 0)
				m_glMapAccess |= GL_MAP_FLUSH_EXPLICIT_BIT;
			if (static_cast<uint32_t>(m_usageFlags & Usage::MAP_COHERENT) > 0)
				m_glMapAccess |= GL_MAP_COHERENT_BIT;

			// Persistent buffers do not need to be unmapped!
			Map();
		}
    }

    Buffer::~Buffer()
    {
		if (m_bufferObject != 0xffffffff)
		{
			if (m_mappedData)
			{
				GL_CALL(glUnmapNamedBuffer, m_bufferObject);

				m_mappedData = nullptr;
			}
			GL_CALL(glDeleteBuffers, 1, &m_bufferObject);
		}
    }

	void* Buffer::Map(GLintptr _offset, GLsizeiptr _numBytes)
    {
        if (m_glMapAccess == 0)
            GLHELPER_LOG_ERROR( "The buffer was not created with READ or WRITE flag. Unable to map memory!" );
		else
		{
			if (m_mappedData != nullptr)
			{
				if (m_mappedDataOffset <= _offset && m_mappedDataOffset + m_mappedDataSize >= _offset + _numBytes)
					return static_cast<char*>(m_mappedData) + _offset;
				else
				{
					GLHELPER_LOG_WARNING("Buffer was already mapped, but within incompatible range. Performing Buffer::Unmap ...");
					Unmap();
				}
			}

			if (m_mappedData == nullptr) // (still) already mapped?
			{
				m_mappedData = GL_RET_CALL(glMapNamedBufferRange, m_bufferObject, _offset, _numBytes, static_cast<GLbitfield>(m_usageFlags & ~Usage::SUB_DATA_UPDATE));

				m_mappedDataOffset = _offset;
				m_mappedDataSize = _numBytes;
			}
		}
		
        return m_mappedData;
    }

    void Buffer::Unmap()
	{
		if (m_mappedData == nullptr)
		{
			GLHELPER_LOG_WARNING("Buffer is not mapped, ignoring unmap operation!");
		}

		// Persistent mapped buffers need no unmapping
		else if (any(m_usageFlags & Usage::MAP_PERSISTENT))
		{
			GLHELPER_LOG_WARNING("Buffer has MAP_PERSISTENT flag and no EXPLICIT_FLUSH flag, unmaps are withouat any effect!");
		}
		else
		{
			GL_CALL(glUnmapNamedBuffer, m_bufferObject);

			m_mappedData = nullptr;
			m_mappedDataOffset = 0;
			m_mappedDataSize = 0;
		}
    }

	void Buffer::Flush()
	{
		Flush(m_mappedDataOffset, m_mappedDataSize);
	}

	void Buffer::Flush(GLintptr _offset, GLsizeiptr _numBytes)
	{
		if (any(m_usageFlags & Usage::EXPLICIT_FLUSH))
		{
			// Flush only the part which was used.
			GL_CALL(glFlushMappedNamedBufferRange, m_bufferObject, _offset, _numBytes);
		}
	}


	void Buffer::Set(const void* _data, GLintptr _offset, GLsizeiptr _numBytes)
    {
		if (any(m_usageFlags & Usage::SUB_DATA_UPDATE))
			GLHELPER_LOG_ERROR("The buffer was not created with the SUB_DATA_UPDATE flag. Unable to set memory!");
		else if (m_mappedData != NULL && (static_cast<GLenum>(m_usageFlags & Usage::MAP_PERSISTENT)))
			GLHELPER_LOG_ERROR("Unable to set memory for currently mapped buffer that was created without the PERSISTENT flag.");
		else {
			GL_CALL(glNamedBufferSubData, m_bufferObject, _offset, _numBytes, _data);
		}
    }

	void Buffer::Get(void* _data, GLintptr _offset, GLsizeiptr _numBytes)
    {
		if (any(m_usageFlags & Usage::SUB_DATA_UPDATE))
			GLHELPER_LOG_ERROR("The buffer was not created with the SUB_DATA_UPDATE flag. Unable to get memory!");
		else if (m_mappedData != NULL && (static_cast<GLenum>(m_usageFlags & Usage::MAP_PERSISTENT)))
			GLHELPER_LOG_ERROR("Unable to get memory for currently mapped buffer that was created without the PERSISTENT flag.");
		else
			GL_CALL(glGetNamedBufferSubData, m_bufferObject, _offset, _numBytes, _data);
    }

	void Buffer::BindVertexBuffer(GLuint _bindingIndex, GLintptr _offset, GLsizeiptr _stride)
	{
		GLHELPER_ASSERT(_bindingIndex < s_numVertexBindings, "Glhelper supports only " + std::to_string(s_numVertexBindings) + 
						" bindings. See glGet with GL_MAX_VERTEX_ATTRIB_BINDINGS for hardware restrictions");
		if (s_boundVertexBuffer[_bindingIndex].buffer != this ||
			s_boundVertexBuffer[_bindingIndex].offset != _offset ||
			s_boundVertexBuffer[_bindingIndex].stride != _stride)
		{
			GL_CALL(glBindVertexBuffer, _bindingIndex, m_bufferObject, _offset, _stride);
			s_boundVertexBuffer[_bindingIndex].buffer = this;
			s_boundVertexBuffer[_bindingIndex].offset = _offset;
			s_boundVertexBuffer[_bindingIndex].stride = _stride;
		}
	}

	void Buffer::BindIndexBuffer()
	{
		if (s_boundIndexBuffer != this)
		{
			GL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, m_bufferObject);
			s_boundIndexBuffer = this;
		}
	}
}
