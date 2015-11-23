#pragma once

#include "gl.hpp"
#include <unordered_map>
#include <cstdint>
#include <functional>

namespace gl
{
	enum class ShaderVariableType
	{
		FLOAT = GL_FLOAT,
		FLOAT_VEC2 = GL_FLOAT_VEC2,
		FLOAT_VEC3 = GL_FLOAT_VEC3,
		FLOAT_VEC4 = GL_FLOAT_VEC4,

		DOUBLE = GL_DOUBLE,
		DOUBLE_VEC2 = GL_DOUBLE_VEC2,
		DOUBLE_VEC3 = GL_DOUBLE_VEC3,
		DOUBLE_VEC4 = GL_DOUBLE_VEC4,

		INT = GL_INT,
		INT_VEC2 = GL_INT_VEC2,
		INT_VEC3 = GL_INT_VEC3,
		INT_VEC4 = GL_INT_VEC4,

		UNSIGNED_INT = GL_UNSIGNED_INT,
		UNSIGNED_INT_VEC2 = GL_UNSIGNED_INT_VEC2,
		UNSIGNED_INT_VEC3 = GL_UNSIGNED_INT_VEC3,
		UNSIGNED_INT_VEC4 = GL_UNSIGNED_INT_VEC4,

		BOOL = GL_BOOL,
		BOOL_VEC2 = GL_BOOL_VEC2,
		BOOL_VEC3 = GL_BOOL_VEC3,
		BOOL_VEC4 = GL_BOOL_VEC4,

		FLOAT_MAT2 = GL_FLOAT_MAT2,
		FLOAT_MAT3 = GL_FLOAT_MAT3,
		FLOAT_MAT4 = GL_FLOAT_MAT4,
		FLOAT_MAT2x3 = GL_FLOAT_MAT2x3,
		FLOAT_MAT2x4 = GL_FLOAT_MAT2x4,
		FLOAT_MAT3x2 = GL_FLOAT_MAT3x2,
		FLOAT_MAT3x4 = GL_FLOAT_MAT3x4,
		FLOAT_MAT4x2 = GL_FLOAT_MAT4x2,
		FLOAT_MAT4x3 = GL_FLOAT_MAT4x3,

		DOUBLE_MAT2 = GL_DOUBLE_MAT2,
		DOUBLE_MAT3 = GL_DOUBLE_MAT3,
		DOUBLE_MAT4 = GL_DOUBLE_MAT4,
		DOUBLE_MAT2x3 = GL_DOUBLE_MAT2x3,
		DOUBLE_MAT2x4 = GL_DOUBLE_MAT2x4,
		DOUBLE_MAT3x2 = GL_DOUBLE_MAT3x2,
		DOUBLE_MAT3x4 = GL_DOUBLE_MAT3x4,
		DOUBLE_MAT4x2 = GL_DOUBLE_MAT4x2,
		DOUBLE_MAT4x3 = GL_DOUBLE_MAT4x3,

		SAMPLER_1D = GL_SAMPLER_1D,
		SAMPLER_2D = GL_SAMPLER_2D,
		SAMPLER_3D = GL_SAMPLER_3D,
		SAMPLER_CUBE = GL_SAMPLER_CUBE,
		SAMPLER_1D_SHADOW = GL_SAMPLER_1D_SHADOW,
		SAMPLER_2D_SHADOW = GL_SAMPLER_2D_SHADOW,
		SAMPLER_1D_ARRAY = GL_SAMPLER_1D_ARRAY,
		SAMPLER_2D_ARRAY = GL_SAMPLER_2D_ARRAY,
		SAMPLER_1D_ARRAY_SHADOW = GL_SAMPLER_1D_ARRAY_SHADOW,
		SAMPLER_2D_ARRAY_SHADOW = GL_SAMPLER_2D_ARRAY_SHADOW,
		SAMPLER_2D_MULTISAMPLE = GL_SAMPLER_2D_MULTISAMPLE,
		SAMPLER_2D_MULTISAMPLE_ARRAY = GL_SAMPLER_2D_MULTISAMPLE_ARRAY,
		SAMPLER_CUBE_SHADOW = GL_SAMPLER_CUBE_SHADOW,
		SAMPLER_BUFFER = GL_SAMPLER_BUFFER,
		SAMPLER_2D_RECT = GL_SAMPLER_2D_RECT,
		SAMPLER_2D_RECT_SHADOW = GL_SAMPLER_2D_RECT_SHADOW,
		INT_SAMPLER_1D = GL_INT_SAMPLER_1D,
		INT_SAMPLER_2D = GL_INT_SAMPLER_2D,
		INT_SAMPLER_3D = GL_INT_SAMPLER_3D,
		INT_SAMPLER_CUBE = GL_INT_SAMPLER_CUBE,
		INT_SAMPLER_1D_ARRAY = GL_INT_SAMPLER_1D_ARRAY,
		INT_SAMPLER_2D_ARRAY = GL_INT_SAMPLER_2D_ARRAY,
		INT_SAMPLER_2D_MULTISAMPLE = GL_INT_SAMPLER_2D_MULTISAMPLE,
		INT_SAMPLER_2D_MULTISAMPLE_ARRAY = GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,
		INT_SAMPLER_BUFFER = GL_INT_SAMPLER_BUFFER,
		INT_SAMPLER_2D_RECT = GL_INT_SAMPLER_2D_RECT,
		UNSIGNED_INT_SAMPLER_1D = GL_UNSIGNED_INT_SAMPLER_1D,
		UNSIGNED_INT_SAMPLER_2D = GL_UNSIGNED_INT_SAMPLER_2D,
		UNSIGNED_INT_SAMPLER_3D = GL_UNSIGNED_INT_SAMPLER_3D,
		UNSIGNED_INT_SAMPLER_CUBE = GL_UNSIGNED_INT_SAMPLER_CUBE,
		UNSIGNED_INT_SAMPLER_1D_ARRAY = GL_UNSIGNED_INT_SAMPLER_1D_ARRAY,
		UNSIGNED_INT_SAMPLER_2D_ARRAY = GL_UNSIGNED_INT_SAMPLER_2D_ARRAY,
		UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE = GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE,
		UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY = GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,
		UNSIGNED_INT_SAMPLER_BUFFER = GL_UNSIGNED_INT_SAMPLER_BUFFER,
		UNSIGNED_INT_SAMPLER_2D_RECT = GL_UNSIGNED_INT_SAMPLER_2D_RECT,

		IMAGE_1D = GL_IMAGE_1D,
		IMAGE_2D = GL_IMAGE_2D,
		IMAGE_3D = GL_IMAGE_3D,
		IMAGE_2D_RECT = GL_IMAGE_2D_RECT,
		IMAGE_CUBE = GL_IMAGE_CUBE,
		IMAGE_BUFFER = GL_IMAGE_BUFFER,
		IMAGE_1D_ARRAY = GL_IMAGE_1D_ARRAY,
		IMAGE_2D_ARRAY = GL_IMAGE_2D_ARRAY,
		IMAGE_2D_MULTISAMPLE = GL_IMAGE_2D_MULTISAMPLE,
		IMAGE_2D_MULTISAMPLE_ARRAY = GL_IMAGE_2D_MULTISAMPLE_ARRAY,
		INT_IMAGE_1D = GL_INT_IMAGE_1D,
		INT_IMAGE_2D = GL_INT_IMAGE_2D,
		INT_IMAGE_3D = GL_INT_IMAGE_3D,
		INT_IMAGE_2D_RECT = GL_INT_IMAGE_2D_RECT,
		INT_IMAGE_CUBE = GL_INT_IMAGE_CUBE,
		INT_IMAGE_BUFFER = GL_INT_IMAGE_BUFFER,
		INT_IMAGE_1D_ARRAY = GL_INT_IMAGE_1D_ARRAY,
		INT_IMAGE_2D_ARRAY = GL_INT_IMAGE_2D_ARRAY,
		INT_IMAGE_2D_MULTISAMPLE = GL_INT_IMAGE_2D_MULTISAMPLE,
		INT_IMAGE_2D_MULTISAMPLE_ARRAY = GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY,
		UNSIGNED_INT_IMAGE_1D = GL_UNSIGNED_INT_IMAGE_1D,
		UNSIGNED_INT_IMAGE_2D = GL_UNSIGNED_INT_IMAGE_2D,
		UNSIGNED_INT_IMAGE_3D = GL_UNSIGNED_INT_IMAGE_3D,
		UNSIGNED_INT_IMAGE_2D_RECT = GL_UNSIGNED_INT_IMAGE_2D_RECT,
		UNSIGNED_INT_IMAGE_CUBE = GL_UNSIGNED_INT_IMAGE_CUBE,
		UNSIGNED_INT_IMAGE_BUFFER = GL_UNSIGNED_INT_IMAGE_BUFFER,
		UNSIGNED_INT_IMAGE_1D_ARRAY = GL_UNSIGNED_INT_IMAGE_1D_ARRAY,
		UNSIGNED_INT_IMAGE_2D_ARRAY = GL_UNSIGNED_INT_IMAGE_2D_ARRAY,
		UNSIGNED_INT_IMAGE_2D_MULTISAMPLE = GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE,
		UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY = GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY,

		UNSIGNED_INT_ATOMIC_COUNTER = GL_UNSIGNED_INT_ATOMIC_COUNTER
	};

	/// Basic information block for Buffer information
	template<typename VariableType>
	struct BufferInfo
	{
		GLint bufferBinding;       ///< OpengGL buffer binding index
		std::int32_t bufferDataSizeByte;  ///< Minimal buffer size in bytes

		/// Known contained variable information
		std::unordered_map<std::string, VariableType> variables;

		// possible, but currently missing:
		// - usage by shader stage (GL_REFERENCED_BY_..)

	private:
		friend class ShaderObject;

		// Internal buffer index for connecting variables with buffers.
		std::int32_t internalBufferIndex;
	};

	/// Basic information block for all possible shader variables.
	struct ShaderVariableInfoBase
	{
		ShaderVariableType  type;        ///< Data type
		std::int32_t		blockOffset; ///< Offset in corresponding buffer, -1 if there's no buffer.

		std::int32_t arrayElementCount; ///< Number of array elements.
		std::int32_t arrayStride;		///< Stride between array elements in bytes.

		std::int32_t matrixStride;  ///< Stride between columns of a column-major matrix or rows of a row-major matrix.
		bool    rowMajor;

		// possible, but currently missing:
		// - usage by shader stage (GL_REFERENCED_BY_..)
	};
	/// Information block for uniform variables
	struct UniformVariableInfo : ShaderVariableInfoBase
	{
		GLint location;      ///< OpenGL location, -1 if used in a buffer.
		GLint atomicCounterbufferIndex; ///< Index of active atomic counter buffer containing this variable?. -1 if this is no atomic counter buffer
	};
	/// Info block for buffer variables
	struct BufferVariableInfo : ShaderVariableInfoBase
	{
		std::int32_t topLevelArraySize;
		std::int32_t topLevelArrayStride;
	};

	typedef BufferInfo<BufferVariableInfo> ShaderStorageBufferMetaInfo;
	typedef BufferInfo<UniformVariableInfo> UniformBufferMetaInfo;


	/// Helper for easy use of buffer meta info.
	///
	/// Can be used to set memory in a buffer by given variables. Note however, that it is always much more efficient to write manually to the corresponding memory.
	template<typename VariableInfoType>
	class BufferInfoView
	{
	public:
		/// Function called for each set command. 
		/// 1) data to set 
		/// 2) size of the data in bytes 
		/// 3) offset of the data within the buffer
		typedef std::function<void(const void* _data, GLsizei _sizeInBytes, std::int32_t _offset)> SetVariableFunction;

		/// Creates a buffer view from buffer meta info and a set variable function.
		BufferInfoView(const BufferInfo<VariableInfoType>& _bufferInfo, const SetVariableFunction& _setVariableFunction) : 
				m_bufferInfo(_bufferInfo), m_setVariableFunction(_setVariableFunction) {}

		class SetableVariable
		{
		public:
			SetableVariable(const VariableInfoType& _metaInfo, const BufferInfoView<VariableInfoType>& _parentBuffer) : m_MetaInfo(_metaInfo), m_parentBuffer(_parentBuffer) {}

			void Set(float f);
			void Set(const gl::Vec2& v);
			void Set(const gl::Vec3& v);
			void Set(const gl::Vec4& v);
			void Set(const gl::Mat3& m);
			void Set(const gl::Mat4& m);

			void Set(double f);
			//void Set(const gl::Vecd& v);
			//void Set(const gl::Vec3d& v);
			//void Set(const gl::Vec4d& v);
			//void Set(const gl::Mat3d& m);
			//void Set(const gl::Mat4d& m);

			void Set(std::uint32_t ui);
			void Set(const gl::UVec2& v);
			void Set(const gl::UVec3& v);
			void Set(const gl::UVec4& v);
			void Set(std::int32_t i);
			void Set(const gl::IVec2& v);
			void Set(const gl::IVec3& v);
			void Set(const gl::IVec4& v);

			// add more type implementations here if necessary

		private:
			void Set(const void* _data, GLsizei _sizeInBytes);

			const VariableInfoType& m_MetaInfo;
			const BufferInfoView<VariableInfoType>& m_parentBuffer;
		};

		bool ContainsVariable(const std::string& _variableName) const   { return m_bufferInfo.variables.find(_variableName) != m_bufferInfo.variables.end(); }
		SetableVariable operator[] (const std::string& _variableName)	{ return SetableVariable(m_bufferInfo.variables.find(_variableName)->second, *this); }


	private:
		friend class SetableVariable;

		const BufferInfo<VariableInfoType>& m_bufferInfo;
		const SetVariableFunction m_setVariableFunction;
	};

	/// A buffer info view, preconfigured for setting variables within a piece of mapped memory.
	template<typename VariableInfoType>
	class MappedMemoryView : public BufferInfoView<VariableInfoType>
	{
	public:
		MappedMemoryView(const BufferInfo<VariableInfoType>& _bufferInfo, void* _mappedMemory, GLsizei _mapOffset = 0);
	};
	
	typedef MappedMemoryView<BufferVariableInfo> MappedBufferView;
	typedef MappedMemoryView<UniformVariableInfo> MappedUBOView;

#include "shaderdatametainfo.inl"
}
