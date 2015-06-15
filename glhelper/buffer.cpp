#include "buffer.hpp"
#include "utils/flagoperators.hpp"

namespace gl
{
	Buffer::BufferBinding Buffer::s_boundVertexBuffers[s_numVertexBufferBindings];
	Buffer::BufferBinding Buffer::s_boundUBOs[s_numUBOBindings];
	Buffer::BufferBinding Buffer::s_boundSSBOs[s_numSSBOBindings];
	BufferId Buffer::s_boundIndexBuffer = 0;
	BufferId Buffer::s_boundIndirectDrawBuffer = 0;
	BufferId Buffer::s_boundIndirectDispatchBuffer = 0;

	Buffer::Buffer(GLsizeiptr _sizeInBytes, UsageFlag _usageFlags, const void* _data) :
        m_sizeInBytes(_sizeInBytes),
		m_usageFlags(_usageFlags),

		m_mappedDataOffset(0),
		m_mappedDataSize(0),
		m_mappedData(nullptr)
    {
		GLHELPER_ASSERT(static_cast<uint32_t>(m_usageFlags & UsageFlag::EXPLICIT_FLUSH) == 0 || static_cast<uint32_t>(m_usageFlags & UsageFlag::MAP_PERSISTENT) > 0,
			   "EXPLICIT_FLUSH only valid in combination with PERSISTENT");
		GLHELPER_ASSERT(static_cast<uint32_t>(m_usageFlags & UsageFlag::MAP_COHERENT) == 0 || static_cast<uint32_t>(m_usageFlags & UsageFlag::MAP_PERSISTENT) > 0,
			   "MAP_COHERENT only valid in combination with PERSISTENT");

		GL_CALL(glCreateBuffers, 1, &m_bufferObject);
		GL_CALL(glNamedBufferStorage, m_bufferObject, _sizeInBytes, _data, static_cast<GLbitfield>(_usageFlags));


		// Persistent buffers do not need to be unmapped!
		if (any(m_usageFlags & UsageFlag::MAP_PERSISTENT))
		{
			MapWriteFlag mapWriteFlags = any(m_usageFlags & UsageFlag::EXPLICIT_FLUSH) ? MapWriteFlag::FLUSH_EXPLICIT : MapWriteFlag::NONE;

			if (any(m_usageFlags & UsageFlag::MAP_WRITE))
				Map(MapType::WRITE, mapWriteFlags);
			else if (any(m_usageFlags & UsageFlag::MAP_READ))
				Map(MapType::READ, mapWriteFlags);
			else if (any(m_usageFlags & UsageFlag::MAP_READ) && any(m_usageFlags & UsageFlag::MAP_WRITE))
				Map(MapType::READ_WRITE, mapWriteFlags);
			else
				LOG_ERROR("Persistently mapped buffers need to specify MAP_READ, MAP_WRITE or both.");
		}
    }

	Buffer::Buffer(Buffer&& _moved) : 
		m_bufferObject(_moved.m_bufferObject),
		m_sizeInBytes(_moved.m_sizeInBytes),
		m_usageFlags(_moved.m_usageFlags),
		m_mappedDataSize(_moved.m_mappedDataSize),
		m_mappedDataOffset(_moved.m_mappedDataOffset),
		m_mappedData(_moved.m_mappedData)
	{
		_moved.m_bufferObject = 0;
		_moved.m_mappedData = nullptr;
	}

    Buffer::~Buffer()
    {
		if (m_bufferObject != 0)
		{
			// According to the specification the buffer will be unmapped automatically on deletion.
			// http://docs.gl/gl4/glUnmapBuffer
			m_mappedData = nullptr;
			/*if (m_mappedData)
			{
				GL_CALL(glUnmapNamedBuffer, m_bufferObject);

				m_mappedData = nullptr;
			}*/

			// According to the specification it is not necessary to unbind the buffer. All bindings reset themselves to zero.
			// http://docs.gl/gl4/glDeleteBuffers
			// However this means, that glhelper's saved bindings are wrong.
			// Iterating over all bindings is rather costly but reliable, easy and zero overhead for all other operations.
			
			if (s_boundIndexBuffer == m_bufferObject)
				s_boundIndexBuffer = 0;

			if (s_boundIndirectDispatchBuffer == m_bufferObject)
				s_boundIndirectDispatchBuffer = 0;

			for (unsigned int i = 0; i < s_numVertexBufferBindings; ++i)
			{
				if (s_boundVertexBuffers[i].bufferObject == m_bufferObject)
					s_boundVertexBuffers[i].bufferObject = 0;
			}

			for (unsigned int i = 0; i < s_numUBOBindings; ++i)
			{
				if (s_boundUBOs[i].bufferObject == m_bufferObject)
					s_boundUBOs[i].bufferObject = 0;
			}
			
			for (unsigned int i = 0; i < s_numSSBOBindings; ++i)
			{
				if (s_boundSSBOs[i].bufferObject == m_bufferObject)
					s_boundSSBOs[i].bufferObject = 0;
			}			

			GL_CALL(glDeleteBuffers, 1, &m_bufferObject);
		}
    }

	void* Buffer::Map(GLintptr _offset, GLsizeiptr _numBytes, MapType _mapType, MapWriteFlag _mapWriteFlags)
    {
		// Check against creation flags.
		GLHELPER_ASSERT(((_mapType == MapType::READ_WRITE || _mapType == MapType::READ) && any(m_usageFlags & UsageFlag::MAP_READ)) ||
						((_mapType == MapType::READ_WRITE || _mapType == MapType::WRITE) && any(m_usageFlags & UsageFlag::MAP_WRITE)), "Can't map the buffer for read/write since it was not created with the read/write usage flags.");

		// Valid range.
		GLHELPER_ASSERT(_offset >= 0 && _numBytes > 0, "Invalid map range!");
		GLHELPER_ASSERT(_numBytes + _offset <= m_sizeInBytes, "Map region exceeds buffer size.");

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
			GLenum accessFlags = 0;

			// Add read/write flags.
			if (_mapType == MapType::READ)
			{
				accessFlags |= GL_MAP_READ_BIT;
			}
			else if (_mapType == MapType::WRITE)
			{
				accessFlags |= GL_MAP_WRITE_BIT;
				accessFlags |= static_cast<GLenum>(_mapWriteFlags);
			}
			else
			{
				GLHELPER_ASSERT(_mapWriteFlags == MapWriteFlag::FLUSH_EXPLICIT || _mapWriteFlags == MapWriteFlag::NONE, "For mapping with both read and write access, the only valid MapWriteFlag is FLUSH_EXPLICIT.");
				
				accessFlags |= GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
				accessFlags |= static_cast<GLenum>(_mapWriteFlags);
			}

			// Add persistent/coherent flags.
			if (any(m_usageFlags & UsageFlag::MAP_PERSISTENT))
			{
				accessFlags |= GL_MAP_PERSISTENT_BIT;
				if (any(m_usageFlags & UsageFlag::MAP_COHERENT))
					accessFlags |= GL_MAP_COHERENT_BIT;
			}

			m_mappedData = GL_RET_CALL(glMapNamedBufferRange, m_bufferObject, _offset, _numBytes, accessFlags);

			m_mappedDataOffset = _offset;
			m_mappedDataSize = _numBytes;
		}
		else
		{
		//	GLHELPER_LOG_WARNING("Buffer was already mapped, but within a compatible range. Returning already mapped pointer ...");
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
		else if (any(m_usageFlags & UsageFlag::MAP_PERSISTENT))
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
		GLHELPER_ASSERT(_numBytes + _offset <= m_sizeInBytes, "Memory range is outside the buffer!");

		if (any(m_usageFlags & UsageFlag::EXPLICIT_FLUSH))
		{
			GL_CALL(glFlushMappedNamedBufferRange, m_bufferObject, _offset, _numBytes);
		}
	}

	void Buffer::ClearToZero()
	{
		GLHELPER_ASSERT(m_mappedData == nullptr, "Can't clear a buffer while it is mapped!");

		GLuint i = 0;
		GL_CALL(glClearNamedBufferData, m_bufferObject, GL_R32UI, GL_RED, GL_UNSIGNED_INT, &i);
	}


	void Buffer::Set(const void* _data, GLintptr _offset, GLsizeiptr _numBytes)
    {
		GLHELPER_ASSERT(_numBytes + _offset <= m_sizeInBytes, "Memory range is outside the buffer!");

		if (any(m_usageFlags & UsageFlag::SUB_DATA_UPDATE))
			GLHELPER_LOG_ERROR("The buffer was not created with the SUB_DATA_UPDATE flag. Unable to set memory!");
		else if (m_mappedData != NULL && (static_cast<GLenum>(m_usageFlags & UsageFlag::MAP_PERSISTENT)))
			GLHELPER_LOG_ERROR("Unable to set memory for currently mapped buffer that was created without the PERSISTENT flag.");
		else {
			GL_CALL(glNamedBufferSubData, m_bufferObject, _offset, _numBytes, _data);
		}
    }

	void Buffer::Get(void* _data, GLintptr _offset, GLsizeiptr _numBytes)
    {
		GLHELPER_ASSERT(_numBytes + _offset <= m_sizeInBytes, "Memory range is outside the buffer!");

		if (any(m_usageFlags & UsageFlag::SUB_DATA_UPDATE))
			GLHELPER_LOG_ERROR("The buffer was not created with the SUB_DATA_UPDATE flag. Unable to get memory!");
		else if (m_mappedData != NULL && (static_cast<GLenum>(m_usageFlags & UsageFlag::MAP_PERSISTENT)))
			GLHELPER_LOG_ERROR("Unable to get memory for currently mapped buffer that was created without the PERSISTENT flag.");
		else
			GL_CALL(glGetNamedBufferSubData, m_bufferObject, _offset, _numBytes, _data);
    }

	void Buffer::BindVertexBuffer(BufferId _buffer, GLuint _bindingIndex, GLintptr _offset, GLsizei _stride)
	{
		GLHELPER_ASSERT(_bindingIndex < s_numVertexBufferBindings, "Glhelper supports only " + std::to_string(s_numVertexBufferBindings) +
			" bindings. See glGet with GL_MAX_VERTEX_ATTRIB_BINDINGS for actual hardware restrictions");

		if (s_boundVertexBuffers[_bindingIndex].bufferObject != _buffer ||
			s_boundVertexBuffers[_bindingIndex].offset != _offset ||
			s_boundVertexBuffers[_bindingIndex].stride != _stride)
		{
			GL_CALL(glBindVertexBuffer, _bindingIndex, _buffer, _offset, _stride);
			s_boundVertexBuffers[_bindingIndex].bufferObject = _buffer;
			s_boundVertexBuffers[_bindingIndex].offset = _offset;
			s_boundVertexBuffers[_bindingIndex].stride = _stride;
		}
	}

	void Buffer::BindIndexBuffer()
	{
		if (s_boundIndexBuffer != m_bufferObject)
		{
			GL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, m_bufferObject);
			s_boundIndexBuffer = m_bufferObject;
		}
	}

	void Buffer::BindIndirectDrawBuffer()
	{
		if (s_boundIndirectDrawBuffer != m_bufferObject)
		{
			GL_CALL(glBindBuffer, GL_DRAW_INDIRECT_BUFFER, m_bufferObject);
			s_boundIndirectDrawBuffer = m_bufferObject;
		}
	}

	void Buffer::BindIndirectDispatchBuffer()
	{
		if (s_boundIndirectDispatchBuffer != m_bufferObject)
		{
			GL_CALL(glBindBuffer, GL_DISPATCH_INDIRECT_BUFFER, m_bufferObject);
			s_boundIndirectDispatchBuffer = m_bufferObject;
		}
	}

	void Buffer::BindUniformBuffer(BufferId _buffer, GLuint _bindingIndex, GLintptr _offset, GLsizeiptr _size)
	{
		GLHELPER_ASSERT(_bindingIndex < s_numUBOBindings, "Glhelper supports only " + std::to_string(s_numUBOBindings) +
			" UBO bindings. See glGet with GL_MAX_UNIFORM_BUFFER_BINDINGS for actual hardware restrictions");

		if (s_boundUBOs[_bindingIndex].bufferObject != _buffer ||
			s_boundUBOs[_bindingIndex].offset != _offset ||
			s_boundUBOs[_bindingIndex].stride != _size)
		{
			GL_CALL(glBindBufferRange, GL_UNIFORM_BUFFER, _bindingIndex, _buffer, _offset, _size);
			s_boundUBOs[_bindingIndex].bufferObject = _buffer;
			s_boundUBOs[_bindingIndex].offset = _offset;
			s_boundUBOs[_bindingIndex].stride = _size;
		}
	}

	void Buffer::BindShaderStorageBuffer(BufferId _buffer, GLuint _bindingIndex, GLintptr _offset, GLsizeiptr _size)
	{
		GLHELPER_ASSERT(_bindingIndex < s_numSSBOBindings, "Glhelper supports only " + std::to_string(s_numSSBOBindings) +
			" UBO bindings. See glGet with GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS for actual hardware restrictions");

		if (s_boundSSBOs[_bindingIndex].bufferObject != _buffer ||
			s_boundSSBOs[_bindingIndex].offset != _offset ||
			s_boundSSBOs[_bindingIndex].stride != _size)
		{
			GL_CALL(glBindBufferRange, GL_SHADER_STORAGE_BUFFER, _bindingIndex, _buffer, _offset, _size);
			s_boundSSBOs[_bindingIndex].bufferObject = _buffer;
			s_boundSSBOs[_bindingIndex].offset = _offset;
			s_boundSSBOs[_bindingIndex].stride = _size;
		}
	}
}
