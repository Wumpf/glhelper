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
	/// This is made possible by ARB_vertex_attrib_binding (core since OpenGL 4.3)
	/// As with most helper classes there are a few restrictions to make the interface easier:
	/// E.g. vertex attributes are not allowed to be overlapping.
	class VertexArrayObject
	{
	public:
		VertexArrayObject(const VertexArrayObject&) = delete;
		void operator = (const VertexArrayObject&) = delete;
		void operator = (VertexArrayObject&&) = delete;

		/// Info about a single vertex attribute.
		///
		/// For details see http://docs.gl/gl4/glVertexAttribFormat
		struct Attribute
		{
			/// Possible element types.
			enum class Type
			{
				INT8,
				UINT8,
				INT16,
				UINT16,
				INT32,
				UINT32,

				FIXED,

				FLOAT,
				HALF,
				DOUBLE, // Uses always glVertexArrayAttribLFormat.

				INT_2_10_10_10 ,
				UINT_2_10_10_10,
				UINT_10F_11F_11F,

				NUM_TYPES
			};

			/// Defines how integer types are interpreted.
			enum class IntegerHandling
			{
				/// Will interpret the data as it is - uint or int depending on Type.
				/// Uses glVertexArrayAttribIFormat internally.
				INTEGER,

				/// Converts to [0;1] for unsigned and to [-1;1] floating point by normalization.
				/// Uses glVertexArrayAttribFormat with normalized=true internally.
				NORMALIZED,

				/// Integer data is directly converted to floating point.
				/// Uses glVertexArrayAttribFormat with normalized=false internally.
				FORCE_FLOAT
			};

			/// Construct attribute. For more details see variable documentation.
			Attribute(Type _type, GLuint _numComponents, GLuint _vertexBufferBinding = 0, IntegerHandling _integerHandling = IntegerHandling::INTEGER, bool _unused = false) :
				type(_type), numComponents(_numComponents), vertexBufferBinding(_vertexBufferBinding), integerHandling(_integerHandling), unused(_unused) {}

			/// The type of a component.
			Type type;
			/// The number of values (see type) per vertex that are stored in the array.
			GLuint numComponents;
			/// The index of the vertex buffer binding with which to associate the generic vertex attribute.
			GLuint vertexBufferBinding;

			/// Describes how integers are handled.
			/// Ignored for float, half and double.
			IntegerHandling integerHandling;

			/// Unused attributes serve as dummy to skip VBO memory.
			/// Note that the corresponding attribute index will stay reserved.
			bool unused;

			/// Assigns each attribute type the corresponding OpenGL type.
			static const GLenum s_typeToGLType[static_cast<int>(Type::NUM_TYPES)];

			/// Assigns each attribute type an OpenGL Type (
			static const GLuint s_typeSizeInBytes[static_cast<int>(Type::NUM_TYPES)];
		};

		/// Constructs a vertex array object from vertex attribute descriptors.
		/// \param _vertexAttributes
		///		List of vertex attributes. Attributes need to be in order as they are occur in the vertex buffer.
		/// \param _divisorIndices
		///		List of binding vertex binding divisors. Position in list gives affected vertex binding.
		///		If divisor value is non-zero, the attributes advance once per divisor instances of the set(s) of vertices being rendered.
		VertexArrayObject(const std::initializer_list<Attribute>& _vertexAttributes, const std::initializer_list<GLuint>& _vertexBindingDivisors = {});
		VertexArrayObject(VertexArrayObject&& _moved);
		~VertexArrayObject();

		/// Binds vertex array if not already bound.
		void Bind();

		/// Resets the VAO binding to 0.
		static void ResetBinding();

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