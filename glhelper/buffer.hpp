#pragma once

#include "gl.hpp"
#include <cstdint>


namespace gl
{
    /// General abstraction for GPU sided memory buffers.
	///
	/// Can be used for UniformBuffer, TextureBuffer, ShaderStorageBuffer, VertexBuffer or IndexBuffer.
	/// Some functionality for specific uses is provided directly (Vertex-, Indexbuffer), other via special view objects (ShaderStorageBuffer, TextureBuffer, UniformBuffer).
	///
	/// Mapping behavior is slightly restricted: Persistent Map-bits are automatically used if specified at creation time.
	///
	/// \see ShaderStorageBufferView, UniformBufferView, TextureBufferView
	///
	/// \todo Better unbinding in destructor - very expensive atm.
	/// \todo Add support for binding multiple vertex buffer at once (glBindVertexBuffers)
    class Buffer
    {
    public:
		friend class UniformBufferView;
		friend class ShaderStorageBufferView;

		Buffer(const Buffer&) = delete;
		void operator = (const Buffer&) = delete;
		void operator = (Buffer&&) = delete;

        /// Buffer creation flags
		/// See flags in http://docs.gl/gl4/glBufferStorage
        enum UsageFlag
        {
			IMMUTABLE = 0,
            MAP_WRITE = GL_MAP_WRITE_BIT,      ///< Enable CPU sided write via mapping.
            MAP_READ = GL_MAP_READ_BIT,       ///< Enable CPU sided read via mapping.
			
			/// Allows the buffer object to be mapped in such a way that it can be used while it is mapped.
			/// The buffer will be mapped entirely once. Calls to map only return the already mapped memory.
			/// See https://www.opengl.org/wiki/Buffer_Object#Persistent_mapping
			/// If set, gl::Buffer will call Map once after creation. Write/Read and Coherent bits are set accordingly to other usage flags.
			/// All calls to Map will automatically contain the GL_MAP_PERSISTENT_BIT.
            MAP_PERSISTENT = GL_MAP_PERSISTENT_BIT,

			/// Only valid in combination with MAP_PERSISTENT. If active, glFlushMappedBufferRange can used instead of barrier.
			EXPLICIT_FLUSH = GL_MAP_FLUSH_EXPLICIT_BIT,

			/// Only valid in combination with MAP_PERSISTENT.
			/// Allows reads from and writes to a persistent buffer to be coherent with OpenGL, without an explicit barrier.
			/// Without this flag, you must use an explicit barrier to achieve coherency. You must use the persistent bit when using this bit.
			/// All calls to Map will automatically contain the GL_MAP_COHERENT_BIT.
            MAP_COHERENT = GL_MAP_COHERENT_BIT,   

            SUB_DATA_UPDATE = GL_DYNAMIC_STORAGE_BIT	///< Makes set and get available
        };

        /// Create and allocate the buffer.
		/// \param _usageFlags
		///		Combination of one or more UsageFlag.
		Buffer(GLsizeiptr _sizeInBytes, UsageFlag _usageFlags, const void* _data = nullptr);
		Buffer(Buffer&& _moved);
        ~Buffer();

        // Clear this buffer
        // Requires format data
        //void Clear(,,,);

		/// Mapping flags for map write.
		/// \see Map, http://docs.gl/gl4/glMapBufferRange
		enum class MapWriteFlag
		{
			NONE = 0,

			INVALIDATE_RANGE = GL_MAP_INVALIDATE_RANGE_BIT,
			INVALIDATE_BUFFER = GL_MAP_INVALIDATE_BUFFER_BIT,

			FLUSH_EXPLICIT = GL_MAP_FLUSH_EXPLICIT_BIT,
			UNSYNCHRONIZED = GL_MAP_UNSYNCHRONIZED_BIT
		};

		/// Possible mapping types.
		enum class MapType
		{
			/// If used for a Map call, the GL_MAP_WRITE_BIT will be added: Returned pointer may be used to modify buffer data.
			READ = 1,
			/// If used for a Map call, the GL_MAP_READ_BIT will be added: Returned pointer may be used to modify buffer data.
			///
			/// Remember not to read any data from a write mapped memory block, since this will have a very high performance penalty!
			/// Sources: https://developer.nvidia.com/content/constant-buffers-without-constant-pain-0, https://msdn.microsoft.com/en-us/library/windows/desktop/ff476457%28v=vs.85%29.aspx
			WRITE = 2,
			/// If used for a Map call, the both GL_MAP_READ_BIT and GL_MAP_WRITE_BIT will be added.
			READ_WRITE = 3
		};

        /// Maps the whole buffer.
		///
		/// \param _writeEnabled
		///		
		/// \param _readEnabled
		///		If true, the GL_MAP_READ_BIT will be added: Returned pointer may be used to read buffer data.
		/// \param _mapWriteFlags
		///		Combination of one or more MapWriteFlag. Ignored if MapType==Write. Attention: The only valid MapWriteFlag is then MapWriteFlag::FLUSH_EXPLICIT
		/// 
		/// \remarks
		///		If usage contains MAP_PERSISTENT/MAP_COHERENT, the corresponding map flags will automatically be used.
		///
		/// \attention
		///		Returns mapped data pointer if entire buffer was already mapped.
		///		First unmaps buffer and writes warning to log if an incompatible area was already mapped.
		/// Performance tips: https://www.opengl.org/wiki/Buffer_Object#Mapping & https://www.opengl.org/wiki/Buffer_Object_Streaming
		void* Map(MapType _mapType, MapWriteFlag _mapWriteFlags) { return Map(0, m_sizeInBytes, _mapType, _mapWriteFlags); }

        /// Maps a part of the buffer.
		///
		/// \see Map
		void* Map(GLintptr _offset, GLsizeiptr _numBytes, MapType _mapType, MapWriteFlag _mapWriteFlags);

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
		/// \see Map, Set
		void Set(const void* _data, GLintptr _offset, GLsizeiptr _numBytes);

        /// Use glGetBufferSubData to update get a data range in the buffer.
		/// \attention
		///		Usage usually not recommend. Consider to use Map.
		void Get(void* _data, GLintptr _offset, GLsizeiptr _numBytes);

		UsageFlag GetUsageFlags() const		{ return m_usageFlags; }
        BufferId GetBufferId() const	{ return m_bufferObject; }
		GLsizeiptr GetSize() const		{ return m_sizeInBytes; }




		/// \see Buffer::BindVertexBuffer
		void BindVertexBuffer(GLuint _bindingIndex, GLintptr _offset, GLsizei _stride);

		/// Binds as vertex buffer on a given slot if not already bound with the same parameters.
		///
		/// Useful in combination with gl::VertexArrayObject. Uses glBindVertexBuffer internally.
		/// \param bindingIndex 
		//		The index of the vertex buffer binding point to which to bind the buffer.
		/// \param offset
		///		The offset of the first element of the buffer.
		/// \param stride
		///		The distance between elements within the buffer.
		static void BindVertexBuffer(BufferId _buffer, GLuint _bindingIndex, GLintptr _offset, GLsizei _stride);


		/// Binds as index buffer (vertex element array) if not already bound with the same parameters.
		void BindIndexBuffer();


		/// \see Buffer::BindUniformBuffer
		void BindUniformBuffer(GLuint _bindingIndex, GLintptr _offset, GLsizeiptr _size);
		/// Binds entire buffer as uniform buffer.
		/// \see Buffer::BindUniformBuffer
		void BindUniformBuffer(GLuint _bindingIndex);

		/// Binds as uniform buffer on a given slot if not already bound with the same parameters.
		///
		/// \param bindingIndex 
		//		The index of the vertex buffer binding point to which to bind the buffer.
		/// \param offset
		///		The offset of the first element of the buffer.
		/// \param stride
		///		The amount of data in machine units that can be read from the buffer object while used as an indexed target.
		static void BindUniformBuffer(BufferId _buffer, GLuint _bindingIndex, GLintptr _offset, GLsizeiptr _size);


		/// \see Buffer::BindShaderStorageBuffer
		void BindShaderStorageBuffer(GLuint _bindingIndex, GLintptr _offset, GLsizeiptr _size);
		/// Binds entire buffer as shader storage buffer.
		/// \see Buffer::BindShaderStorageBuffer
		void BindShaderStorageBuffer(GLuint _bindingIndex);

		/// Binds as uniform buffer on a given slot if not already bound with the same parameters.
		///
		/// \param bindingIndex 
		//		The index of the vertex buffer binding point to which to bind the buffer.
		/// \param offset
		///		The offset of the first element of the buffer.
		/// \param size
		///		The amount of data in machine units that can be read from the buffer object while used as an indexed target.
		static void BindShaderStorageBuffer(BufferId _buffer, GLuint _bindingIndex, GLintptr _offset, GLsizeiptr _size);

    private:
		friend class PersistentRingBuffer;

        BufferId m_bufferObject;
		GLsizeiptr m_sizeInBytes;
        UsageFlag m_usageFlags;			///< The newer glMapBufferRange requires the same bits as the construction

		GLsizeiptr m_mappedDataSize;
		GLintptr m_mappedDataOffset;
		void* m_mappedData;

		// -----------------
		// Redundant binding checks.

		/// Struct for general buffer bindings.
		struct BufferBinding
		{
			BufferBinding() : bufferObject(0), offset(0), stride(0) {}

			/// The bound buffer object.
			BufferId bufferObject;
			/// The offset of the first element of the buffer.
			GLintptr offset;
			/// The distance between elements within the buffer.
			/// Might also be used as "size".
			GLsizeiptr stride;
		};

		// Vertex Buffer
		static const unsigned int s_numVertexBufferBindings = 16;
		static BufferBinding s_boundVertexBuffers[s_numVertexBufferBindings];

		// Index Buffer
		static BufferId s_boundIndexBuffer;

		// Uniform buffer
		static const unsigned int s_numUBOBindings = 64;	/// Arbitrary value based on observation: http://delphigl.de/glcapsviewer/gl_stats_caps_single.php?listreportsbycap=GL_MAX_COMBINED_UNIFORM_BLOCKS
		static BufferBinding s_boundUBOs[s_numUBOBindings];

		// Shader Storage buffer
		static const unsigned int s_numSSBOBindings = 16; /// Arbitrary value, based on observation: http://delphigl.de/glcapsviewer/gl_stats_caps_single.php?listreportsbycap=GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS
		static BufferBinding s_boundSSBOs[s_numSSBOBindings];
    };

	#include "buffer.inl"
}