#include "vertexarrayobject.hpp"
#include <vector>


namespace gl
{
	/// Assigns each attribute type the corresponding OpenGL type.
	const GLenum VertexArrayObject::Attribute::s_typeToGLType[static_cast<int>(VertexArrayObject::Attribute::Type::NUM_TYPES)] =
	{
		GL_BYTE,
		GL_UNSIGNED_BYTE,
		GL_SHORT,
		GL_UNSIGNED_SHORT,
		GL_INT,
		GL_UNSIGNED_INT,

		GL_FIXED,
		GL_FLOAT,
		GL_HALF_FLOAT,
		GL_DOUBLE,

		GL_INT_2_10_10_10_REV,
		GL_UNSIGNED_INT_2_10_10_10_REV,
		GL_UNSIGNED_INT_10F_11F_11F_REV,
	};

	const GLuint VertexArrayObject::Attribute::s_typeSizeInBytes[static_cast<int>(VertexArrayObject::Attribute::Type::NUM_TYPES)] =
	{
		1, // INT8
		1, // UINT8
		2, // INT16
		2, // UINT16
		4, // INT32
		4, // UINT32

		4, // FIXED

		4, // FLOAT
		2, // HALF
		8, // DOUBLE

		4, // INT_2_10_10_10
		4, // UINT_2_10_10_10
		4, // UINT_10F_11F_11F
	};


	VertexArrayObject* VertexArrayObject::s_boundVertexArray = nullptr;

	VertexArrayObject::VertexArrayObject(const std::initializer_list<Attribute>& _vertexAttributes, const std::initializer_list<GLuint>& _vertexBindingDivisors) :
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
			if (!attribute.unused)
			{
				GL_CALL(glEnableVertexArrayAttrib, m_vao, attributeIndex);
				GL_CALL(glVertexArrayAttribBinding, m_vao, attributeIndex, attribute.vertexBufferBinding);

				// Define attribute properties.
				// Double has extra function:
				if (attribute.type == Attribute::Type::DOUBLE)
				{
					GL_CALL(glVertexArrayAttribLFormat, m_vao, attributeIndex, attribute.numComponents, Attribute::s_typeToGLType[static_cast<int>(attribute.type)], m_vertexStrides[attribute.vertexBufferBinding]);
				}
				// All other as float interpreted data.
				else if (attribute.type == Attribute::Type::FLOAT || attribute.type == Attribute::Type::HALF ||
					attribute.type == Attribute::Type::FIXED || attribute.type == Attribute::Type::UINT_10F_11F_11F ||
					attribute.integerHandling != Attribute::IntegerHandling::INTEGER)
				{
					GL_CALL(glVertexArrayAttribFormat, m_vao, attributeIndex, attribute.numComponents, Attribute::s_typeToGLType[static_cast<int>(attribute.type)],
						attribute.integerHandling == Attribute::IntegerHandling::NORMALIZED, m_vertexStrides[attribute.vertexBufferBinding]);
				}
				// Integer interpretation.
				else
				{
					GL_CALL(glVertexArrayAttribIFormat, m_vao, attributeIndex, attribute.numComponents, Attribute::s_typeToGLType[static_cast<int>(attribute.type)], m_vertexStrides[attribute.vertexBufferBinding]);
				}
			}
			else
			{
				GL_CALL(glDisableVertexArrayAttrib, m_vao, attributeIndex);
			}

			// Advance offset.
			m_vertexStrides[attribute.vertexBufferBinding] += attribute.numComponents * Attribute::s_typeSizeInBytes[static_cast<int>(attribute.type)];
		}

		auto divisorIterator = _vertexBindingDivisors.begin();;
		for (GLuint bindingIndex = 0; bindingIndex < _vertexBindingDivisors.size(); ++bindingIndex, ++divisorIterator)
		{
			GL_CALL(glVertexArrayBindingDivisor, m_vao, bindingIndex, *divisorIterator);
		}
	}

	VertexArrayObject::VertexArrayObject(VertexArrayObject&& _moved) :
		m_vao(_moved.m_vao),
		m_vertexAttributes(std::move(_moved.m_vertexAttributes)),
		m_vertexStrides(std::move(_moved.m_vertexStrides))
	{
		_moved.m_vao = 0;
	}

	VertexArrayObject::~VertexArrayObject()
	{
		if(m_vao != 0)
		{
			if(s_boundVertexArray == this)
				ResetBinding();

			GL_CALL(glDeleteVertexArrays, 1, &m_vao);
		}
	}

	void VertexArrayObject::Bind()
	{
		if (s_boundVertexArray != this)
		{
			GL_CALL(glBindVertexArray, m_vao);
			s_boundVertexArray = this;
		}
	}

	void VertexArrayObject::ResetBinding()
	{
		s_boundVertexArray = nullptr;
		GL_CALL(glBindVertexArray, 0);
	}

	GLuint VertexArrayObject::GetVertexStride(GLuint _vertexBufferSlotIndex) const
	{
		if (_vertexBufferSlotIndex >= m_vertexStrides.size())
			return 0;
		else
			return m_vertexStrides[_vertexBufferSlotIndex];
	}
}