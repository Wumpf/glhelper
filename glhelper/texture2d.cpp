#include "texture2d.hpp"
#include <string>

#ifdef TEXTURE2D_FROMFILE_STBI
// Tells stb to implement all image definitions in this file.
#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>
#endif

namespace gl
{
	Texture2D::Texture2D(GLsizei _width, GLsizei _height, TextureFormat _format, GLsizei _numMipLevels, GLsizei _numMSAASamples) :
		Texture(_width, _height, 1, _format, _numMipLevels, _numMSAASamples)
	{
		GL_CALL(glCreateTextures, GetOpenGLTextureType(), 1, &m_textureHandle);
		if(m_numMSAASamples == 0)
			GL_CALL(glTextureStorage2D, m_textureHandle, m_numMipLevels, gl::TextureFormatToGLSizedInternal[static_cast<unsigned int>(_format)], m_width, m_height);
		else
			GL_CALL(glTextureStorage2DMultisample, m_textureHandle, m_numMSAASamples, gl::TextureFormatToGLSizedInternal[static_cast<unsigned int>(_format)], m_width, m_height, GL_FALSE);
	}

	Texture2D::Texture2D(GLsizei _width, GLsizei _height, TextureFormat _format, const void* _data, TextureSetDataFormat _dataFormat, TextureSetDataType _dataType, GLsizei _numMipLevels, GLsizei _numMSAASamples) :
		Texture2D(_width, _height, _format, _numMipLevels, _numMSAASamples)
	{
		GL_CALL(glTextureSubImage2D, m_textureHandle, 0, 0, 0, m_width, m_height,
				static_cast<GLenum>(_dataFormat), static_cast<GLenum>(_dataType), _data);
	}

#ifdef TEXTURE2D_FROMFILE_STBI
	std::unique_ptr<Texture2D> Texture2D::LoadFromFile(const std::string& _filename, bool _generateMipMaps, bool _sRGB)
	{
		int texSizeX = -1;
		int texSizeY = -1;

		int numComps = -1;
		stbi_uc* textureData = stbi_load(_filename.c_str(), &texSizeX, &texSizeY, &numComps, 4);
		if (!textureData)
		{
			GLHELPER_LOG_ERROR("Error loading texture \"" + _filename + "\".");
			return nullptr;
		}

		std::unique_ptr<Texture2D> newTex(new Texture2D(static_cast<GLsizei>(texSizeX), static_cast<GLsizei>(texSizeY), _sRGB ? gl::TextureFormat::SRGB8_ALPHA8 : gl::TextureFormat::RGBA8, _generateMipMaps ? 0 : 1));
		newTex->SetData(0, TextureSetDataFormat::RGBA, TextureSetDataType::UNSIGNED_BYTE, textureData);
		if (_generateMipMaps)
			newTex->GenMipMaps();

		stbi_image_free(textureData);

		return std::move(newTex);
	}
#endif

	void Texture2D::SetData(GLsizei _mipLevel, TextureSetDataFormat _dataFormat, TextureSetDataType _dataType, const void* _data, const gl::UVec2& _areaOffset, const gl::UVec2& _areaSize)
	{
		GLHELPER_ASSERT(_mipLevel >= 0, "Invalid mipmap level");
		GLHELPER_ASSERT(_mipLevel < m_numMipLevels, "MipLevel " + std::to_string(_mipLevel) + " does not exist, texture has only " + std::to_string(m_numMipLevels) + " MipMapLevels");
		GLHELPER_ASSERT(_areaSize.x != 0 && _areaSize.y != 0, "SetData area is zero sized!");
		GLHELPER_ASSERT(_areaOffset.x < static_cast<decltype(_areaOffset.x)>(m_width >> _mipLevel) &&
						_areaOffset.y < static_cast<decltype(_areaOffset.y)>(m_height >> _mipLevel), "SetData area offset is outside of the texture!");
		GLHELPER_ASSERT(_areaSize.x + _areaOffset.x <= static_cast<decltype(_areaOffset.x)>(m_width >> _mipLevel) &&
						_areaSize.y + _areaOffset.y <= static_cast<decltype(_areaOffset.y)>(m_height >> _mipLevel), "SetData area overwrite overlaps regions outside the texture!");

		GL_CALL(glTextureSubImage2D, m_textureHandle, _mipLevel, _areaOffset.x, _areaOffset.y, _areaSize.x, _areaSize.y, static_cast<GLenum>(_dataFormat), static_cast<GLenum>(_dataType), _data);
	}
}