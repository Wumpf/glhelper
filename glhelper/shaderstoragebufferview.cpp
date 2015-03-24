#include "shaderstoragebufferview.hpp"

namespace gl
{
	ShaderStorageBufferView::ShaderStorageBufferView(std::shared_ptr<gl::Buffer> _buffer, const std::string& _name)
	{
		m_buffer = _buffer;
		m_name = _name;
	}

	ShaderStorageBufferView::~ShaderStorageBufferView()
	{
	}
}