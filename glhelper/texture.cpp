#include "texture.hpp"

namespace gl
{
	TextureId Texture::s_boundTextures[Texture::s_numTextureBindings];

	Texture::Texture(GLsizei width, GLsizei height, GLsizei depth, TextureFormat format, GLsizei numMipLevels, GLsizei numMSAASamples) :
		m_width(width),
		m_height(height),
		m_depth(depth),

		m_format(format),
		m_numMipLevels(ConvertMipMapSettingToActualCount(numMipLevels, width, height, depth)),

		m_numMSAASamples(numMSAASamples)
	{
		GLHELPER_ASSERT(m_width > 0 || m_height > 0 || m_depth > 0, "Invalid texture size.");
		GLHELPER_ASSERT(m_numMipLevels > 0, "Invalid mipmap level count.");

		GLHELPER_ASSERT(m_numMipLevels == 1 || numMSAASamples == 0, "Texture must have either zero MSAA samples or only one miplevel!");
	}

	Texture::Texture(Texture&& _moved) :
		m_textureHandle(_moved.m_textureHandle),

		m_width(_moved.m_width),
		m_height(_moved.m_height),
		m_depth(_moved.m_depth),

		m_format(_moved.m_format),
		m_numMipLevels(_moved.m_numMipLevels),
		m_numMSAASamples(_moved.m_numMSAASamples)
	{
		_moved.m_textureHandle = 0;
	}

	Texture::~Texture()
	{
		// According to the specification it is not necessary to unbind the texture. All bindings reset themselves to zero.
		// http://docs.gl/gl4/glDeleteTextures
		// However this means, that glhelper's saved bindings are wrong!
		// Since they contain texture handles, this might result in rejected binding of new textures!

		// Iterating over all bindings is rather costly but reliable, easy and zero overhead for all other operations.
		for (unsigned int i = 0; i < s_numTextureBindings; ++i)
		{
			if (s_boundTextures[i] == m_textureHandle)
				s_boundTextures[i] = 0;
		}

		GL_CALL(glDeleteTextures, 1, &m_textureHandle);
	}

	GLsizei Texture::ConvertMipMapSettingToActualCount(GLsizei _mipMapSetting, GLsizei _width, GLsizei _height, GLsizei _depth)
	{
		if (_mipMapSetting == 0)
		{
			GLsizei numMipLevels = 0;
			while (_width > 0 || _height > 0 || _depth > 0)
			{
				_width /= 2;
				_height /= 2;
				_depth /= 2;
				++numMipLevels;
			}
			return numMipLevels;
		}

		else
			return _mipMapSetting;
	}

	void Texture::BindImage(GLuint _slotIndex, Texture::ImageAccess access, TextureFormat format)
	{
		GL_CALL(glBindImageTexture, _slotIndex, m_textureHandle, 0, GL_TRUE, 0, static_cast<GLenum>(access), gl::TextureFormatToGLSizedInternal[static_cast<unsigned int>(format)]);
	}

	void Texture::ResetImageBinding(GLuint _slotIndex)
	{
		GL_CALL(glBindImageTexture, _slotIndex, 0, 0, false, 0, GL_READ_WRITE, GL_RGBA8);
	}

    void Texture::ResetBinding(GLuint _slotIndex)
	{
		GLHELPER_ASSERT(_slotIndex < sizeof(s_boundTextures) / sizeof(Texture*), "Can't bind texture to slot " + std::to_string(_slotIndex) + 
							". Maximum number of slots in glhelper is " + (std::to_string(sizeof(s_boundTextures) / sizeof(Texture*)) + 
							". For actual hardware restrictions see glGet GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS"));
		if (s_boundTextures[_slotIndex] != 0)
		{
			GL_CALL(glBindTextureUnit, _slotIndex, 0);
			s_boundTextures[_slotIndex] = 0;
		}
	}

	void Texture::ClearToZero(GLsizei _mipLevel)
	{
		GLHELPER_ASSERT(m_numMipLevels > _mipLevel, "Miplevel " + std::to_string(_mipLevel) + " not available, texture has only " + std::to_string(m_numMipLevels) + " levels!");

		GL_CALL(glClearTexImage, m_textureHandle, _mipLevel, gl::TextureFormatToGLBaseInternal[static_cast<unsigned int>(m_format)], GL_UNSIGNED_INT, nullptr);
	}

	void Texture::Bind(GLuint _slotIndex)
	{
		Bind(m_textureHandle, _slotIndex);
	}

	void Texture::Bind(TextureId textureHandle, GLuint _slotIndex)
	{
		GLHELPER_ASSERT(_slotIndex < sizeof(s_boundTextures) / sizeof(void*), "Can't bind texture to slot " + std::to_string(_slotIndex) + ". Maximum number of slots is " + std::to_string(sizeof(s_boundTextures) / sizeof(Texture*)));
		if(s_boundTextures[_slotIndex] != textureHandle)
		{
			GL_CALL(glBindTextureUnit, _slotIndex, textureHandle);
			s_boundTextures[_slotIndex] = textureHandle;
		}
	}

	void Texture::ReadImage(GLsizei _mipLevel, TextureReadFormat _format, TextureReadType _type, GLsizei _bufferSize, void* _buffer)
	{
		GLHELPER_ASSERT(m_numMipLevels > _mipLevel, "Miplevel " + std::to_string(_mipLevel) + " not available, texture has only " + std::to_string(m_numMipLevels) + " levels!");
		GL_CALL(glGetTextureImage, m_textureHandle, _mipLevel, static_cast<GLenum>(_format), static_cast<GLenum>(_type), _bufferSize, _buffer);
	}

	void Texture::GenMipMaps()
	{
		GL_CALL(glGenerateTextureMipmap, m_textureHandle);
	}
}