#pragma once

#include "gl.hpp"
#include "shaderdatametainfo.hpp"
#include "buffer.hpp"

#include <initializer_list>
#include <algorithm>
#include <memory>

namespace gl
{
	/// Allows easy use of gl::Buffer as a uniform buffer.
	///
	/// UniformBufferViews have names for usage with gl::ShaderObject reflected meta data.
	class UniformBufferView
	{
	public:
		/// Init as view to existing buffer.
		///
		/// Buffer needs to have at least MAP_WRITE access!
		UniformBufferView(const std::shared_ptr<Buffer>& _buffer, const std::string& _bufferName);

		/// Creates a new buffer without any meta infos.
		///
		/// Buffer needs to have at least MAP_WRITE access!
		UniformBufferView(std::uint32_t _bufferSizeBytes, const std::string& _bufferName, Buffer::UsageFlag bufferUsage = Buffer::UsageFlag::MAP_WRITE);

		/// Create a uniform buffer that matches the meta infos of a given shader.
		UniformBufferView(const gl::ShaderObject& _shader, const std::string& _bufferName, Buffer::UsageFlag bufferUsage = Buffer::UsageFlag::MAP_WRITE);

		~UniformBufferView();

		class Variable : public gl::ShaderVariable < UniformVariableInfo >
		{
		public:
			Variable() : ShaderVariable(), m_uniformBuffer(NULL) { }
			Variable(const UniformVariableInfo& metaInfo, UniformBufferView* pUniformBuffer) :
				ShaderVariable(metaInfo), m_uniformBuffer(pUniformBuffer) {}
			

			void Set(const void* pData, GLsizei SizeInBytes) override;

			using gl::ShaderVariable<UniformVariableInfo>::Set;
		private:
			UniformBufferView* m_uniformBuffer;
		};


		/// Sets variable on currently mapped data block.
		///
		/// \attention
		///		Checks only via GLHELPER_ASSERT if memory area is actually mapped!
		void Set(const void* _data, GLsizei offset, GLsizei _sizeInBytes);

		bool ContainsVariable(const std::string& _variableName) const       { return m_variables.find(_variableName) != m_variables.end(); }
		UniformBufferView::Variable operator[] (const std::string& sVariableName);

		/// Binds entire buffer if not already bound.
		void BindBuffer(GLuint _locationIndex) const { m_buffer->BindUniformBuffer(_locationIndex); }

		const std::string& GetBufferName() const { return m_bufferName; }

		/// Get the internal bound buffer resource.
		const std::shared_ptr<Buffer>& GetBuffer() const { return m_buffer; }

	private:
		void InitByCreatingBuffer(std::uint32_t _bufferSizeBytes, const std::string& _bufferName, Buffer::UsageFlag _bufferUsage);

		std::shared_ptr<Buffer> m_buffer;
		std::string		m_bufferName;

		/// meta information
		std::unordered_map<std::string, gl::UniformVariableInfo> m_variables;
	};

#include "UniformBuffer.inl"
}
