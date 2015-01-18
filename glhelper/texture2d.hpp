#pragma once

#include "texture.hpp"
#include <memory>
#include <string>

namespace gl
{
	class Texture2D : public Texture
	{
	public:
		/// New empty 2d texture.
		///
		/// \param _numMipLevels
		///		Number of MipMap levels. If 0, the maximum possible number of mipmaps will be determined and used.
		Texture2D(GLsizei _width, GLsizei _height, TextureFormat _format, GLsizei _numMipLevels = 1, GLsizei _numMSAASamples = 0);

		/// New single miplevel 2d texture with given data.
		///
		/// \param _numMipLevels
		///		Number of MipMap levels. If 0, the maximum possible number of mipmaps will be determined and used.
		Texture2D(GLsizei _width, GLsizei _height, TextureFormat _format, const void* _data, TextureSetDataFormat _dataFormat, TextureSetDataType _dataType, GLsizei _numMSAASamples = 0);

		Texture2D(Texture2D&& _moved) : Texture(std::move(_moved)) {}

#ifdef TEXTURE2D_FROMFILE_STBI
		/// \brief Loads texture from file using stb_image
		///
		/// Uses given to create an object if file loading is successful. Use corresponding deallocator to remove the texture.
		/// Will return NULL on error.
		static std::unique_ptr<Texture2D> LoadFromFile(const std::string& _filename, bool _generateMipMaps = true, bool _sRGB = false);
#endif

		/// Overwrites all data of a given mip level.
		void SetData(GLsizei _mipLevel, TextureSetDataFormat _dataFormat, TextureSetDataType _dataType, const void* _data)
		{ SetData(_mipLevel, _dataFormat, _dataType, _data, gl::UVec2(0, 0), gl::UVec2(m_width, m_height)); }

		/// Overwrites data of a given mip level within the given area.
		/// 
		/// \param _areaOffset
		///		Offset of the area from (0,0) in pixel coordinates.
		/// \param _areaSize
		///		Size of the area in pixels which will be overwritten by _data.
		void SetData(GLsizei _mipLevel, TextureSetDataFormat _dataFormat, TextureSetDataType _dataType, const void* _data, const gl::UVec2& _areaOffset, const gl::UVec2& _areaSize);

		GLenum GetOpenGLTextureType() const override { return GetNumMSAASamples() > 0 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D; }
	};

}

