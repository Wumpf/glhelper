#include "uniformbufferview.hpp"
#include "shaderobject.hpp"
#include "buffer.hpp"
#include "utils/flagoperators.hpp"

namespace gl
{
	BufferId UniformBufferView::s_boundUBOs[s_numUBOBindings];

	UniformBufferView::~UniformBufferView()
	{
		// According to the specification it is not necessary to unbind the buffer. All bindings reset themselves to zero.
		// http://docs.gl/gl4/glDeleteBuffers (performed by gl::Buffer)
		// However this means, that glhelper's saved bindings are wrong.
		// Iterating over all bindings is rather costly but reliable, easy and zero overhead for all other operations.
		for (unsigned int i = 0; i < s_numUBOBindings; ++i)
		{
			if (s_boundUBOs[i] == m_buffer->GetBufferId())
				s_boundUBOs[i] = 0;
		}
	}

	UniformBufferView::UniformBufferView(const std::shared_ptr<Buffer>& _buffer, const std::string& _bufferName)
	{
		if (static_cast<std::uint32_t>(_buffer->GetUsageFlags() & Buffer::Usage::MAP_WRITE))
		{
			GLHELPER_LOG_WARNING("Uniform buffer needs at least Buffer::Usage::WRITE flag to work as expected!");
		}
		else
		{
			m_bufferName = _bufferName;
			m_buffer = _buffer;
		}
	}

	UniformBufferView::UniformBufferView(std::uint32_t _bufferSizeBytes, const std::string& _bufferName, Buffer::Usage _bufferUsage)
	{
		InitByCreatingBuffer(_bufferSizeBytes, _bufferName, _bufferUsage);
	}

	UniformBufferView::UniformBufferView(const gl::ShaderObject& _shader, const std::string& _bufferName, Buffer::Usage _bufferUsage)
	{
		GLuint bufferSize = 0;

		// Check if uniform exists in this shader.
		auto uniformBufferInfoIterator = _shader.GetUniformBufferInfo().find(_bufferName);
		if (uniformBufferInfoIterator == _shader.GetUniformBufferInfo().end())
		{
			GLHELPER_LOG_ERROR("ShaderObject \"" + _shader.GetName() + "\" in list for uniform buffer \"" + _bufferName + "\" initialization doesn't contain the needed meta data! Skiping..");
		}
		else
		{
			m_variables = uniformBufferInfoIterator->second.Variables;
			InitByCreatingBuffer(uniformBufferInfoIterator->second.bufferDataSizeByte, _bufferName, _bufferUsage);
		}
	}

	void UniformBufferView::InitByCreatingBuffer(std::uint32_t _bufferSizeBytes, const std::string& _bufferName, Buffer::Usage _bufferUsage)
	{
		if (static_cast<std::uint32_t>(_bufferUsage & Buffer::Usage::MAP_WRITE) == 0)
		{
			GLHELPER_LOG_WARNING("Uniform buffer needs at least Buffer::Usage::WRITE flag to work as expected!");
		}
		else
		{
			m_bufferName = _bufferName;
			m_buffer.reset(new Buffer(_bufferSizeBytes, _bufferUsage));
		}
	}

	void UniformBufferView::BindBuffer(GLuint locationIndex)
	{
		GLHELPER_ASSERT(locationIndex < s_numUBOBindings, 
			"Can't bind ubo to slot " + std::to_string(locationIndex) + ". Maximum number of slots is " + std::to_string(s_numUBOBindings));

		if (m_buffer->m_mappedData != nullptr && static_cast<GLenum>(m_buffer->m_usageFlags & Buffer::Usage::MAP_PERSISTENT) == 0)
			m_buffer->Unmap();

		if (s_boundUBOs[locationIndex] != m_buffer->GetBufferId())
		{
			GL_CALL(glBindBufferBase, GL_UNIFORM_BUFFER, locationIndex, m_buffer->GetBufferId());
			s_boundUBOs[locationIndex] = m_buffer->GetBufferId();
		}

	}

	void UniformBufferView::Set(const void* _data, GLsizei _offset, GLsizei _dataSize)
	{
		GLHELPER_ASSERT(m_buffer != nullptr, "Uniform buffer " << m_bufferName << " is not initialized");
		GLHELPER_ASSERT(_dataSize != 0, "Given size to set for uniform data is 0.");
		GLHELPER_ASSERT(_offset + _dataSize <= m_buffer->GetSize(), "Data block doesn't fit into uniform buffer.");
		GLHELPER_ASSERT(_data != nullptr, "Data to copy into uniform is nullptr.");


		GLHELPER_ASSERT(m_buffer->m_mappedData != nullptr, "Buffer is not mapped!");
		GLHELPER_ASSERT(m_buffer->m_mappedDataOffset <= _offset && m_buffer->m_mappedDataOffset + m_buffer->m_mappedDataSize >= _dataSize + _offset, "Buffer mapping range is not sufficient.");

		memcpy(static_cast<std::uint8_t*>(m_buffer->m_mappedData) + _offset, reinterpret_cast<const char*>(_data), _dataSize);
	}

}