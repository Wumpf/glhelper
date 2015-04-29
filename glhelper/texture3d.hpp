#pragma once

#include "texture.hpp"
#include <memory>
#include <string>

namespace gl
{
	class Texture3D : public Texture
	{
	public:
		/// New empty 3d texture.
		///
		/// \param _numMipLevels
		///		Number of MipMap levels. If 0, the maximum possible number of mipmaps will be determined and used.
		Texture3D(GLsizei _width, GLsizei _height, GLsizei _depth, TextureFormat _format, GLsizei _numMipLevels = 1);

		/// Overwrites all data of a given mip level.
		void SetData(GLsizei _mipLevel, TextureSetDataFormat _dataFormat, TextureSetDataType _dataType, const void* _data)
		{ SetData(_mipLevel, _dataFormat, _dataType, _data, gl::UVec3(0, 0, 0), gl::UVec3(m_width, m_height, m_depth)); }

		/// Overwrites data of a given mip level within the given volume.
		/// 
		/// \param _volumeOffset
		///		Offset of the area from (0,0,0) in pixel coordinates.
		/// \param _volumeSize
		///		Size of the area in pixels which will be overwritten by _data.
		void SetData(GLsizei _mipLevel, TextureSetDataFormat _dataFormat, TextureSetDataType _dataType, const void* _data, const gl::UVec3& _volumeOffset, const gl::UVec3& _volumeSize);

		GLenum GetOpenGLTextureType() const override { return GL_TEXTURE_3D; }
	};

}

