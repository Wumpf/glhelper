#pragma once

#include "gl.hpp"
#include "shaderdatametainfo.hpp"
#include "buffer.hpp"

#include <initializer_list>
#include <algorithm>
#include <memory>
#include <queue>

namespace gl
{
	/// Helper class for write-only GPU ring buffer.
	///
	/// This class is especially useful for the common use case of frequently changing UBOs (~ once per draw).
	/// A large gpu buffer is allocated once and mapped persistently with EXPLICIT_FLUSH (no coherent flag).
	/// The user can now add blocks, freely write them and bind them after flushing. After a block was bound, it should no longer be written to, since the GPU might use it!
	/// 
	/// All blocks are associated with the current frame. For each new frame, all old buffers are no longer accessible and a new fence will be created.
	/// Every time the user wants adds a block, the class will check if the memory lies in a former frame. In this case the class will wait for the frame's fence to complete.
	///
	/// Typical use case for per-object-UBO:
	/// Create a PersistentRingBuffer with size = #ExpectedObjects * 3. At the begin of the frame call "CompleteFrame".
	/// For each object call AddBlock and fill up the memory. Then call FlushAllBlocks to make sure the memory is visible to the GPU.
	/// Now you can freely bind the blocks to use them by the GPU.
	class PersistentRingBuffer
	{
	public:
		/// Allocates a gl::Buffer with size _sizeInBytes and gl::Buffer::Usage::MAP_WRITE, MAP_PERSISTENT and EXPLICIT_FLUSH.
		PersistentRingBuffer(GLsizeiptr _sizeInBytes);

		/// Adds a new memory block for writing.
		///
		/// Will make sure that the returned memory is currently not used by the GPU.
		/// This means that this function will stall if the GPU still uses memory needed for this block. If this happens consider to create a larger buffer.
		///
		/// \param _outMemory
		///		Pointer to memory for writing. It is ensure that the GPU does not use this memory block after this call.
		///		You need to be very careful with it: Accidental reads are extremely costly. Call flush if you are done with memory operations.
		///		Remember that you should not use the memory block after you handed it over to any GPU command.
		/// \param _outBlockIndex
		///		Block index used for FlushBlockRange and Bind functions. Used to identify the memory range.
		/// \param _alignment
		///		Enforces a given byte alignment. For uniform buffers for example you need to use glGet(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT).
		///		Zero means no alignment.
		void AddBlock(void*& _outMemory, size_t& outBlockIndex, unsigned int _sizeInBytes, unsigned int _alignment = 0);

		/// Flushes the memory of all blocks.
		///
		/// You need to perform a flush after memory writes to ensure writes are visible to the GPU.
		/// \see FlushBlockRange
		void FlushAllBlocks() { FlushBlockRange(0, m_blockList.size() - 1);  }

		/// Flushes all blocks in a consecutive range.
		///
		/// You need to perform a flush after memory writes to ensure writes are visible to the GPU.
		/// \param _startBlock
		///		First block to be flushed.
		/// \param _endBlock
		///		Last consecutive block to be flushed. Must be larger or equal to _startBlock
		/// \see FlushAllBlocks
		void FlushBlockRange(size_t _startBlock, size_t _endBlock) { m_buffer.Flush(m_blockList[_startBlock].start, m_blockList[_endBlock].size); }

		/// Orphans all blocks and adds a fence to the internal fence queue.
		///
		/// You need to call this function when you are finished with all commands that use the currently set blocks.
		/// Subsequent AddBlock commands will make sure that the returned memory is not in use by any GL command preceding this call.
		void CompleteFrame();


		/// Returns underlying gl::Buffer.
		gl::Buffer& GetBuffer()				{ return m_buffer; }
		/// Returns underlying gl::Buffer.
		const gl::Buffer& GetBuffer() const { return m_buffer; }

		// -----------------------------
		// On ensuring adding blocks without wait.

		/// Enables or disables warnings for sync events within AddBlock.
		/// Default is true.
		void SetWarnOnWaitForSync(bool _warnOnSyncWait) { m_warnOnSyncWait = _warnOnSyncWait; }
		
		/// Returns the number of frames that the GPU can read from until the current frame will be reached.
		///
		/// If the returned value is permanently equal or smaller than one, following calls of AddBlock might stall.
		size_t GetNumberOfPendingFrames() { return m_frameQueue.size() - 1; }


		/// Returns the sync timeout in nanoseconds.
		/// 
		/// Default is 1 second.
		void SetSyncTimeoutNanoseconds(GLuint64 syncTimeoutNS) { m_syncTimeOut = syncTimeoutNS; }

		/// Returns the sync timeout in nanoseconds.
		GLuint64 GetSyncTimeoutNanoseconds() { return m_syncTimeOut; }

		// -----------------------------

		// Various binding operations. Remember to flush the corresponding block before binding it!

		/// Binds a given block as a uniform buffer Object.
		/// 
		/// Remember to manually flush the block after any memory modifications before binding it! Any writes after a bind operation result in undefined behavior.
		/// Though you are still allowed to bind the block to another location.
		/// \param _UBOlocationIndex
		///		The UBO binding point to bind the memory to.
		void BindBlockAsUBO(GLuint _UBOlocationIndex, unsigned int _blockIndex)
		{ GLHELPER_ASSERT(_blockIndex < m_blockList.size(), "Invalid block index"); m_buffer.BindUniformBuffer(_UBOlocationIndex, m_blockList[_blockIndex].start, m_blockList[_blockIndex].size); }

		/// \todo
		//void BindBlockAsSSBO(unsigned int blockIndex);

		/// \todo
		//void BindBlockAsVertexBuffer(unsigned int blockIndex);

		/// \todo
		//void BindBlockAsIndexBuffer(unsigned int blockIndex);

	private:
		gl::Buffer m_buffer;

		
		/// Saving both start and size for a block seems to be redundant, but you need to note that it might happen that (BufferSize-NextWrite % BlockSize != 0)
		/// This means that there might be unused memory at the end of the buffer since blocks are not allowed to be part at the end part at the start of the buffer.
		/// size+start must thus guaranteed to be <BufferSize
		struct Block
		{
			unsigned int start;
			unsigned int size;
		};
		std::vector<Block> m_blockList;
		unsigned int m_nextWritePosition; ///< Next byte to be allocated by the next block.

		
		struct Sync
		{
			Sync(unsigned int _startMemoryPosition) : startMemoryPosition(_startMemoryPosition), fence(0) {}

			GLsync fence;
			unsigned int startMemoryPosition; ///< First byte belonging to this frame.
		};
		/// The first element is always the currently active frame.
		std::queue<Sync> m_frameQueue;

		GLuint64 m_syncTimeOut;

		bool m_warnOnSyncWait;
	};
}
