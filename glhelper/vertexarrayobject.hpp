#pragma once

#include "gl.hpp"
#include <cstdint>
#include <vector>

namespace gl
{
	/// Abstraction of OpenGL vertex array objects (VAO).
	///
	/// This helper class is restricted to the use of VAOs as pure vertex definition,
	/// which means that it does NOT reference any buffer (vertex array or vertex element array) buffers.
	/// As with most helper classes there are some restrictions to make the interface easier:
	/// E.g. vertex attributes are not allowed to be overlapping.
	/// This is made possible by ARB_vertex_attrib_binding (core since OpenGL 4.3)
	///
	/// \todo Divisor rates for instancing (http://docs.gl/gl4/glVertexBindingDivisor)
	class VertexArrayObject
	{
	public:
		/// Info about a single vertex attribute.
		struct Attribute
		{
			/// Possible element types.
			enum class Type : GLenum
			{
				INT8 = GL_BYTE,
				UINT8 = GL_UNSIGNED_BYTE,
				INT16 = GL_SHORT,
				UINT16 = GL_UNSIGNED_SHORT,
				INT32 = GL_INT,
				UINT32 = GL_UNSIGNED_INT,

				FIXED = GL_FIXED,

				FLOAT = GL_FLOAT,
				HALF = GL_HALF_FLOAT,
				DOUBLE = GL_DOUBLE,

				INT_2_10_10_10 = GL_INT_2_10_10_10_REV,
				UINT_2_10_10_10 = GL_UNSIGNED_INT_2_10_10_10_REV,
				UINT_10F_11F_11F = GL_UNSIGNED_INT_10F_11F_11F_REV,

				NUM_TYPES
			};

			/// Construct attribute. For more details see variable documentation.
			Attribute(Type _type, GLuint _numComponents, GLuint _vertexBufferBinding = 0, bool _normalized = false) :
				type(_type), numComponents(_numComponents), vertexBufferBinding(_vertexBufferBinding), normalized(_normalized) {}

			/// The type of a component.
			Type type;
			/// The number of values (see type) per vertex that are stored in the array.
			GLuint numComponents;
			/// The index of the vertex buffer binding with which to associate the generic vertex attribute.
			GLuint vertexBufferBinding;
	
			/// If true, converts to [0;1] for unsigned and to [-1;1] floating point by normalization.
			/// Flag is ignored for floats and doubles.
			bool normalized;
		};

		/// Constructs a vertex array object from vertex attribute descriptors.
		/// \param vertexAttributes List of vertex attributes. Attributes need to be in order as they are occur in the vertex buffer.
		VertexArrayObject(const std::initializer_list<Attribute>& _vertexAttributes);
		~VertexArrayObject();

		/// Binds vertex array if not already bound.
		void BindVertexArray();

		/// Returns intern OpenGL vertex array object handle.
		VertexArrayObjectId GetInternHandle() const { return m_vao; }

		/// Returns the vertex stride for a given vertex buffer slot.
		///
		/// Values were computed from the Vertex-Attribute info on creation.
		/// \param vertexBufferSlotIndex 
		/// \return Stride in bytes. Zero if there is no information about the given vertexBufferSlotIndex.
		GLuint GetVertexStride(GLuint _vertexBufferSlotIndex = 0) const;

		/// Returns vertex attribute description that was passed during construction.
		const std::vector<Attribute>& GetVertexAttributeDesc() const { return m_vertexAttributes; }

	private:
		/// Currently bound vertex array.
		static VertexArrayObject* s_boundVertexArray;

		/// OpenGL handle.
		VertexArrayObjectId m_vao;

		/// Vertex attribute description.
		std::vector<Attribute> m_vertexAttributes;

		/// Vertex size per vertex buffer slot.
		std::vector<GLuint> m_vertexStrides;
	};
}