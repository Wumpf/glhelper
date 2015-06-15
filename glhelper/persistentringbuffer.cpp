#include "persistentringbuffer.hpp"
#include "utils/flagoperators.hpp"

namespace gl
{
	PersistentRingBuffer::PersistentRingBuffer(GLsizeiptr _sizeInBytes) :
		m_buffer(_sizeInBytes, Buffer::MAP_WRITE | Buffer::MAP_PERSISTENT | Buffer::EXPLICIT_FLUSH),
		m_syncTimeOut(1000000000), // One second timeout.
		m_warnOnSyncWait(true)
	{
		m_frameQueue.emplace(0);
		m_buffer.Map(Buffer::MapType::WRITE, Buffer::MapWriteFlag::FLUSH_EXPLICIT);
	}

	void PersistentRingBuffer::AddBlock(void*& _outMemory, size_t& _outBlockIndex, unsigned int _sizeInBytes, unsigned int _alignment)
	{
		GLHELPER_ASSERT(_sizeInBytes < static_cast<unsigned int>(m_buffer.GetSize()), "Block is larger than the entire ring buffer!");

		_outBlockIndex = m_blockList.size();

		// First get actual memory position. Might be different if remaining buffer if we need to restart with the buffer.
		m_blockList.emplace_back();
		Block& newBlock = m_blockList.back();
		newBlock.size = _sizeInBytes;
		newBlock.start = m_nextWritePosition + (_alignment - (m_nextWritePosition % _alignment)) % _alignment; // Need to follow alignment rules.
		unsigned int startWithoutAlignment = m_nextWritePosition;

		bool skippedMem = false;
		unsigned int remainingMem = static_cast<unsigned int>(m_buffer.GetSize()) - newBlock.start;
		if (remainingMem < _sizeInBytes)
		{
			newBlock.start = 0;
			startWithoutAlignment = 0;
			skippedMem = true;
		}
		unsigned int blockEndExclusive = newBlock.start + _sizeInBytes;

		// Range of [startWithoutAlignment; blockEndExclusive[ may be larger.
		// If any frame start lies within, wait for its end!
		// Additionally if we skipped memory at the end of the buffer, we need to wait for that too.
		while (m_frameQueue.size() > 1 && 
				((m_frameQueue.front().startMemoryPosition >= startWithoutAlignment && m_frameQueue.front().startMemoryPosition < blockEndExclusive) || // Frame starts within this block.
				(skippedMem && m_nextWritePosition <= m_frameQueue.front().startMemoryPosition)))
		{
			// GL_SYNC_FLUSH_COMMANDS_BIT ensures that the sync object is already in the command queue.
			// Without this, the function might endup in a endless loop. According to OpenGL Super Bible (5th edition page 534) there is usally no reason not to add this flag.
			GLenum syncState = GL_RET_CALL(glClientWaitSync, m_frameQueue.front().fence, GL_SYNC_FLUSH_COMMANDS_BIT, m_syncTimeOut);
			if (syncState == GL_WAIT_FAILED)
			{
				_outMemory = nullptr;
				gl::CheckGLError("glClientWaitSync");
				return;
			}
			else if (syncState == GL_TIMEOUT_EXPIRED)
			{
				_outMemory = nullptr;
				GLHELPER_LOG_ERROR("Timeout for glClientWaitSync on RingBuffer memory has expired!");
				return;
			}
			else if (syncState == GL_CONDITION_SATISFIED && m_warnOnSyncWait)
			{
				GLHELPER_LOG_WARNING("GPU/CPU sync occured during PersistentRingBuffer::AddBlock. Consider to use a larger ringerbuffer.");
			}

			GL_CALL(glDeleteSync, m_frameQueue.front().fence);
			m_frameQueue.pop();
		}

		// Bit in our own tail = contains start of the first block of this frame?
		if (m_blockList.size() > 1 && m_blockList.front().start >= newBlock.start && m_blockList.front().start < blockEndExclusive)
		{
			_outMemory = nullptr;
			GLHELPER_LOG_ERROR("The ring buffer does not contain enough memory to hold a single frame. Usually you should overprovise by a factor of 3!");
			return;
		}

		// Advance write position.
		m_nextWritePosition = (m_blockList.back().start + m_blockList.back().size) % m_buffer.GetSize();
		
		// Now we are safe to use the memory.
		_outMemory = static_cast<char*>(m_buffer.m_mappedData) + m_blockList.back().start;
	}

	void PersistentRingBuffer::CompleteFrame()
	{
		if (m_blockList.empty())
		{
			GLHELPER_LOG_ERROR("No new ringbuffer block was created since the last call of CompleteFrame");
			return;
		}

		// Create fence.
		m_frameQueue.back().fence = GL_RET_CALL(glFenceSync, GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

		// Start new frame.
		m_frameQueue.emplace(m_nextWritePosition);
		m_blockList.clear();
	}
}