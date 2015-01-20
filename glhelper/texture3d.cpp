#include "texture3d.hpp"

namespace gl
{
	Texture3D::Texture3D(GLsizei width, GLsizei height, GLsizei _depth, TextureFormat format, GLsizei numMipLevels) :
		Texture(width, height, _depth, format, numMipLevels)
	{
		GL_CALL(glCreateTextures, GL_TEXTURE_3D, 1, &m_textureHandle);
		GL_CALL(glTextureStorage3D, m_textureHandle, m_numMipLevels, gl::TextureFormatToGLSizedInternal[static_cast<unsigned int>(format)], m_width, m_height, m_depth);
	}

	void Texture3D::SetData(GLsizei _mipLevel, TextureSetDataFormat _dataFormat, TextureSetDataType _dataType, const void* _data, const gl::UVec3& _volumeOffset, const gl::UVec3& _volumeSize)
	{
		GLHELPER_ASSERT(_mipLevel >= 0, "Invalid mipmap level");
		GLHELPER_ASSERT(_mipLevel < m_numMipLevels, "MipLevel " + std::to_string(_mipLevel) + " does not exist, texture has only " + std::to_string(m_numMipLevels) + " MipMapLevels");
		GLHELPER_ASSERT(_volumeSize.x != 0 && _volumeSize.y != 0, "SetData volume is zero sized!");
		GLHELPER_ASSERT(_volumeOffset.x < static_cast<decltype(_volumeOffset.x)>(m_width >> _mipLevel) &&
						_volumeOffset.y < static_cast<decltype(_volumeOffset.y)>(m_height >> _mipLevel) &&
						_volumeOffset.z < static_cast<decltype(_volumeOffset.z)>(m_depth >> _mipLevel), "SetData volume offset is outside of the texture!");
		GLHELPER_ASSERT(_volumeSize.x + _volumeOffset.x <= static_cast<decltype(_volumeOffset.x)>(m_width >> _mipLevel) &&
						_volumeSize.y + _volumeOffset.y <= static_cast<decltype(_volumeOffset.y)>(m_height >> _mipLevel) &&
						_volumeSize.z + _volumeOffset.z <= static_cast<decltype(_volumeOffset.z)>(m_depth >> _mipLevel), "SetData volume overwrite overlaps regions outside the texture!");

		GL_CALL(glTextureSubImage3D, m_textureHandle, _mipLevel, _volumeOffset.x, _volumeOffset.y, _volumeOffset.z,
					_volumeSize.x, _volumeSize.y, _volumeSize.z, static_cast<GLenum>(_dataFormat), static_cast<GLenum>(_dataType), _data);
	}
}