#include "uniformbuffer.hpp"
#include "shaderobject.hpp"
#include "buffer.hpp"
#include "utils/flagoperators.hpp"

namespace gl
{
	BufferId UniformBufferView::s_boundUBOs[s_numUBOBindings];

	UniformBufferView::UniformBufferView() :
		m_variables(),
		m_bufferName("")
	{
	}

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

	Result UniformBufferView::Init(std::shared_ptr<Buffer> _buffer, const std::string& _bufferName)
	{
		if (static_cast<std::uint32_t>(_buffer->GetUsageFlags() & Buffer::Usage::MAP_WRITE))
		{
			GLHELPER_LOG_ERROR("Uniform buffer need at least Buffer::Usage::WRITE!");
			return gl::Result::FAILURE;
		}
		else
		{
			m_bufferName = _bufferName;
			m_buffer = _buffer;
			return gl::Result::SUCCEEDED;
		}
	}

	Result UniformBufferView::Init(std::uint32_t _bufferSizeBytes, const std::string& _bufferName, Buffer::Usage _bufferUsage)
	{
		if (static_cast<std::uint32_t>(_bufferUsage & Buffer::Usage::MAP_WRITE) == 0)
		{
			GLHELPER_LOG_ERROR("Uniform buffer need at least Buffer::Usage::WRITE!");
			return gl::Result::FAILURE;
		}
		else
		{
			m_bufferName = _bufferName;
			m_buffer.reset(new Buffer(_bufferSizeBytes, _bufferUsage));
			return gl::Result::SUCCEEDED;
		}
	}

	Result UniformBufferView::Init(const gl::ShaderObject& _shader, const std::string& _bufferName, Buffer::Usage _bufferUsage)
	{
		auto uniformBufferInfoIterator = _shader.GetUniformBufferInfo().find(_bufferName);
		if (uniformBufferInfoIterator == _shader.GetUniformBufferInfo().end())
		{
			GLHELPER_LOG_ERROR("Shader \"" + _shader.GetName() + "\" doesn't contain a uniform buffer meta block info with the name \"" + _bufferName + "\"!");
			return gl::Result::FAILURE;
		}

		for (auto it = uniformBufferInfoIterator->second.Variables.begin(); it != uniformBufferInfoIterator->second.Variables.end(); ++it)
			m_variables.emplace(it->first, Variable(it->second, this));

		return Init(uniformBufferInfoIterator->second.iBufferDataSizeByte, _bufferName, _bufferUsage);
	}

	Result UniformBufferView::Init(std::initializer_list<const gl::ShaderObject*> metaInfos, const std::string& bufferName, Buffer::Usage bufferUsage)
	{
		GLHELPER_ASSERT(metaInfos.size() != 0, "Meta info lookup list is empty!");

		bool initialized = false;
		int i = 0;
		for (auto shaderObjectIt = metaInfos.begin(); shaderObjectIt != metaInfos.end(); ++shaderObjectIt, ++i)
		{
			GLHELPER_ASSERT(*shaderObjectIt != nullptr, "Metainfo element is nullptr!");
			auto uniformBufferInfoIterator = (*shaderObjectIt)->GetUniformBufferInfo().find(bufferName);
			if (uniformBufferInfoIterator == (*shaderObjectIt)->GetUniformBufferInfo().end())
			{
				GLHELPER_LOG_WARNING("ShaderObject \"" + (*shaderObjectIt)->GetName() + "\" in list for uniform buffer \"" + bufferName + "\" initialization doesn't contain the needed meta data! Skiping..");
				continue;
			}

			if (!initialized)
			{
				Result result = Init(**metaInfos.begin(), bufferName, bufferUsage);
				if (result == Result::FAILURE)
					initialized = true;
				
				continue;
			}

			// Sanity check.
			if (uniformBufferInfoIterator->second.iBufferDataSizeByte != m_buffer->GetSize())
			{
				GLHELPER_LOG_WARNING("ShaderObject \"" << (*shaderObjectIt)->GetName() << "\" in list for uniform buffer \"" << bufferName << "\" initialization gives size " <<
						uniformBufferInfoIterator->second.iBufferDataSizeByte << ", first shader gave size " << m_buffer->GetSize() << "! Skiping..");
				continue;
			}

			for (auto varIt = uniformBufferInfoIterator->second.Variables.begin(); varIt != uniformBufferInfoIterator->second.Variables.end(); ++varIt)
			{
				auto ownVarIt = m_variables.find(varIt->first);
				if (ownVarIt != m_variables.end())  // overlap
				{
					// Sanity check.
					const gl::UniformVariableInfo* ownVar = &ownVarIt->second.GetMetaInfo();
					const gl::UniformVariableInfo* otherVar = &varIt->second;
					if (memcmp(ownVar, otherVar, sizeof(gl::UniformVariableInfo)) != 0)
					{
						GLHELPER_LOG_ERROR("ShaderObject \"" + (*shaderObjectIt)->GetName() + "\" in list for uniform buffer \"" + bufferName + "\" has a description of variable \"" +
							varIt->first + "\" that doesn't match with the ones before!");
					}
				}
				else // new one
				{
					m_variables.emplace(varIt->first, Variable(varIt->second, this));
					// No overlap checking so far.
				}
			}
		}

		return m_buffer != nullptr ? Result::SUCCEEDED : Result::FAILURE;
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

	void UniformBufferView::Set(const void* _data, std::uint32_t _offset, std::uint32_t _dataSize)
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