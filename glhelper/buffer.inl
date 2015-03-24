
inline void Buffer::BindVertexBuffer(GLuint _bindingIndex, GLintptr _offset, GLsizei _stride)
{
	GLHELPER_ASSERT((static_cast<unsigned int>(m_usageFlags)& gl::Buffer::UsageFlag::MAP_PERSISTENT) || m_mappedData == nullptr,
					"Only persistent buffers can be bound while beeing mapped.");
	Buffer::BindVertexBuffer(m_bufferObject, _bindingIndex, _offset, _stride);
}

inline void Buffer::BindUniformBuffer(GLuint _bindingIndex, GLintptr _offset, GLsizeiptr _size)
{
	GLHELPER_ASSERT((static_cast<unsigned int>(m_usageFlags)& gl::Buffer::UsageFlag::MAP_PERSISTENT) || m_mappedData == nullptr,
		"Only persistent buffers can be bound while beeing mapped.");
	Buffer::BindUniformBuffer(m_bufferObject, _bindingIndex, _offset, _size);
}

inline void Buffer::BindUniformBuffer(GLuint _bindingIndex)
{
	GLHELPER_ASSERT((static_cast<unsigned int>(m_usageFlags)& gl::Buffer::UsageFlag::MAP_PERSISTENT) || m_mappedData == nullptr,
		"Only persistent buffers can be bound while beeing mapped.");
	Buffer::BindUniformBuffer(m_bufferObject, _bindingIndex, 0, m_sizeInBytes);
}

inline void Buffer::BindShaderStorageBuffer(GLuint _bindingIndex, GLintptr _offset, GLsizeiptr _size)
{
	GLHELPER_ASSERT((static_cast<unsigned int>(m_usageFlags)& gl::Buffer::UsageFlag::MAP_PERSISTENT) || m_mappedData == nullptr,
		"Only persistent buffers can be bound while beeing mapped.");
	Buffer::BindShaderStorageBuffer(m_bufferObject, _bindingIndex, _offset, _size);
}

inline void Buffer::BindShaderStorageBuffer(GLuint _bindingIndex)
{
	GLHELPER_ASSERT((static_cast<unsigned int>(m_usageFlags)& gl::Buffer::UsageFlag::MAP_PERSISTENT) || m_mappedData == nullptr,
		"Only persistent buffers can be bound while beeing mapped.");
	Buffer::BindShaderStorageBuffer(m_bufferObject, _bindingIndex, 0, m_sizeInBytes);
}