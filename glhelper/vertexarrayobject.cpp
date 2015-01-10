#include "vertexarrayobject.hpp"
#include <vector>

namespace gl
{
	VertexArrayObject* VertexArrayObject::s_boundVertexArray = nullptr;

	VertexArrayObject::VertexArrayObject(const std::initializer_list<Attribute>& _vertexAttributes) :
		m_vertexAttributes(_vertexAttributes)
	{
		GL_CALL(glCreateVertexArrays, 1, &m_vao);

		for (GLuint attributeIndex = 0; attributeIndex<m_vertexAttributes.size(); ++attributeIndex)
		{
			const Attribute& attribute = m_vertexAttributes[attributeIndex];

			// Debug checks
			GLHELPER_ASSERT((attribute.numComponents > 0 && attribute.numComponents <= 4) || attribute.numComponents == GL_BGRA, "Invalid vertex attribute component number!");
			GLHELPER_ASSERT(attribute.numComponents == 1 || 
							(attribute.type != Attribute::Type::INT_2_10_10_10 && attribute.type != Attribute::Type::UINT_2_10_10_10 &&attribute.type != Attribute::Type::UINT_10F_11F_11F),
							"Num vertex components needs to be 1 for packed formats");

			while (attribute.vertexBufferBinding >= m_vertexStrides.size())
				m_vertexStrides.push_back(0);

			// Activate and define vertex buffer binding point.
			GL_CALL(glEnableVertexArrayAttrib, m_vao, attributeIndex);
			GL_CALL(glVertexArrayAttribBinding, m_vao, attributeIndex, attribute.vertexBufferBinding);

			// Define attribute properties.
			if (attribute.type == Attribute::Type::DOUBLE)
			{
				GL_CALL(glVertexArrayAttribLFormat, m_vao, attributeIndex, attribute.numComponents, GL_DOUBLE, m_vertexStrides[attribute.vertexBufferBinding]);
				m_vertexStrides[attribute.vertexBufferBinding] += attribute.numComponents * sizeof(double);
			}
			else
			{
				GL_CALL(glVertexArrayAttribFormat, m_vao, attributeIndex, attribute.numComponents, static_cast<GLenum>(attribute.type), attribute.normalized, m_vertexStrides[attribute.vertexBufferBinding]);
				m_vertexStrides[attribute.vertexBufferBinding] += attribute.numComponents * 4;
			}

			++attributeIndex;
		}
	}

	VertexArrayObject::~VertexArrayObject()
	{
		GL_CALL(glDeleteVertexArrays, 1, &m_vao);
	}

	void VertexArrayObject::BindVertexArray()
	{
		if (s_boundVertexArray != this)
		{
			GL_CALL(glBindVertexArray, m_vao);
			s_boundVertexArray = this;
		}
	}

	GLuint VertexArrayObject::GetVertexStride(GLuint _vertexBufferSlotIndex) const
	{
		if (_vertexBufferSlotIndex >= m_vertexStrides.size())
			return 0;
		else
			return m_vertexStrides[_vertexBufferSlotIndex];
	}
}