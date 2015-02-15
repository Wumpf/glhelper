#pragma once

#include "gl.hpp"

#include <cstdint>

namespace gl
{
    /// General abstraction for GPU sided memory buffers.
	///
	/// Can be used for UniformBuffer, TextureBuffer, ShaderStorageBuffer, VertexBuffer or IndexBuffer.
	/// Some functionality for specific uses is provided directly (vertex-, indexbuffer), other via special view objects (ShaderStorageBuffer, TextureBuffer, UniformBuffer).
	/// \see ShaderStorageBufferView, UniformBufferView, TextureBufferView
	///
	/// \todo Maps currently use creation-time defined mapping flag instead of user given flag.
	/// \todo Add support for binding multiple vertex buffer at once (glBindVertexBuffers)
    class Buffer
    {
    public:
		friend class UniformBufferView;
		friend class ShaderStorageBufferView;

		Buffer(const Buffer&) = delete;
		void operator = (const Buffer&) = delete;
		void operator = (Buffer&&) = delete;

        /// Flags for the (optimized) buffer creation.
        enum class Usage
        {
			IMMUTABLE = 0,
            MAP_WRITE = GL_MAP_WRITE_BIT,      ///< Enable CPU sided write via mapping.
            MAP_READ = GL_MAP_READ_BIT,       ///< Enable CPU sided read via mapping.
			
			/// Allows the buffer object to be mapped in such a way that it can be used while it is mapped.
			/// The buffer will be mapped entirely once. Calls to map only return the already mapped memory.
			/// See https://www.opengl.org/wiki/Buffer_Object#Persistent_mapping
            MAP_PERSISTENT = GL_MAP_PERSISTENT_BIT, 
			/// Only valid in combination with MAP_PERSISTENT. If active, glFlushMappedBufferRange can used instead of barrier.
			EXPLICIT_FLUSH = GL_MAP_FLUSH_EXPLICIT_BIT,
			/// Only valid in combination with MAP_PERSISTENT.
			/// Allows reads from and writes to a persistent buffer to be coherent with OpenGL, without an explicit barrier.
			/// Without this flag, you must use an explicit barrier to achieve coherency. You must use the persistent bit when using this bit.
            MAP_COHERENT = GL_MAP_COHERENT_BIT,   

            SUB_DATA_UPDATE = GL_DYNAMIC_STORAGE_BIT	///< Makes set and get available
        };

        /// Create and allocate the buffer
		Buffer(GLsizeiptr _sizeInBytes, Usage _usageFlags, const void* _data = nullptr);
		Buffer(Buffer&& _moved);
        ~Buffer();

        // Clear this buffer
        // Requires format data
        //void Clear(,,,);

        /// Maps the whole buffer.
		///
		/// \attention
		///		Returns mapped data pointer if entire buffer was already mapped. Unmaps buffer and writes warning to log if an incompatible area was already mapped.
		/// Performance: https://www.opengl.org/wiki/Buffer_Object#Mapping
		/// Map should be at least as fast as SubData, but is expected to be better.
		/// However writing sequentially is appreciated (as always with any piece of memory!).
		void* Map() { return Map(0, m_sizeInBytes); }

        /// Maps a part of the buffer.
		///
		/// \attention
		///		Returns mapped data pointer if desired buffer area was already mapped.
		///		Unmaps buffer and writes warning to log if an incompatible area was already mapped.
		/// \see Map
		void* Map(GLintptr _offset, GLsizeiptr _numBytes);

		/// Unmaps the buffer.
		///
		/// Writes a warning to the log and ignores call if:
		/// * ..buffer was not mapped.
		/// * ..buffer was created with MAP_PERSISTENT, since buffer can stay mapped virtually forever. 
		/// (Later means that you have to take care of synchronizations yourself. 
		/// Either via glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT) or glFlushMappedBufferRange.)
        void Unmap();

		/// Explicit flush as long as EXPLICIT_FLUSH flag is set. Otherwise this function has no effect.
		void Flush();
		void Flush(GLintptr _offset, GLsizeiptr _numBytes);

		/// Clears buffer to zero using glClearNamedBufferData (http://docs.gl/gl4/glClearBufferData)
		void ClearToZero();

        /// Use glBufferSubData to update a range in the buffer.
		/// \remarks
		///		The Map methods are usually faster.
		/// \see Map, Set
		void Set(const void* _data, GLintptr _offset, GLsizeiptr _numBytes);

        /// Use glGetBufferSubData to update get a data range in the buffer.
		/// \attention
		///		Usage usually not recommend. Consider to use Map.
		void Get(void* _data, GLintptr _offset, GLsizeiptr _numBytes);

		Usage GetUsageFlags() const		{ return m_usageFlags; }
        BufferId GetBufferId() const	{ return m_bufferObject; }
		GLsizeiptr GetSize() const		{ return m_sizeInBytes; }


		/// Binds as vertex buffer on a given slot if not already bound with the same parameters.
		///
		/// Useful in combination with gl::VertexArrayObject.
		/// Uses glBindVertexBuffer internally.
		/// \param bindingIndex 
		//		The index of the vertex buffer binding point to which to bind the buffer.
		/// \param offset
		///		The offset of the first element of the buffer.
		/// \param stride
		///		The distance between elements within the buffer.
		void BindVertexBuffer(GLuint _bindingIndex, GLintptr _offset, GLsizei _stride);

		/// Binds as index buffer (vertex element array) if not already bound with the same parameters.
		void BindIndexBuffer();

    private:
        BufferId m_bufferObject;
		GLsizeiptr m_sizeInBytes;
        GLenum m_glMapAccess;   ///< READ_ONLY, WRITE_ONLY or READ_WRITE, and optionally GL_MAP_PERSISTENT_BIT.
        Usage m_usageFlags;			///< The newer glMapBufferRange requires the same bits as the construction

		GLsizeiptr m_mappedDataSize;
		GLintptr m_mappedDataOffset;
		void* m_mappedData;


		/// Descriptor for a vertex buffer binding.
		struct VertexBufferBinding
		{
			VertexBufferBinding() : bufferObject(0), offset(0), stride(0) {}

			/// The bound buffer object.
			BufferId bufferObject;
			/// The offset of the first element of the buffer.
			GLintptr offset;
			/// The distance between elements within the buffer.
			GLsizei stride;
		};
		static const unsigned int s_numVertexBufferBindings = 16;
		static VertexBufferBinding s_boundVertexBuffers[s_numVertexBufferBindings];
		static BufferId s_boundIndexBuffer;
    };
}