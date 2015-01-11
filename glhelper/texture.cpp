#include "texture.hpp"

namespace gl
{
	TextureId Texture::s_boundTextures[Texture::s_numTextureBindings];

	Texture::Texture(std::uint32_t width, std::uint32_t height, std::uint32_t depth, TextureFormat format, std::int32_t numMipLevels, std::uint32_t numMSAASamples) :
		m_width(width),
		m_height(height),
		m_depth(depth),

		m_format(format),
		m_numMipLevels(ConvertMipMapSettingToActualCount(numMipLevels, width, height, depth)),

		m_numMSAASamples(numMSAASamples)
	{
		GLHELPER_ASSERT(m_numMipLevels == 1 || numMSAASamples == 0, "Texture must have either zero MSAA samples or only one miplevel!");
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

	std::uint32_t Texture::ConvertMipMapSettingToActualCount(std::int32_t iMipMapSetting, std::uint32_t width, std::uint32_t height, std::uint32_t depth)
	{
		if (iMipMapSetting <= 0)
		{
			std::uint32_t uiNumMipLevels = 0;
			while (width > 0 || height > 0 || depth > 0)
			{
				width /= 2;
				height /= 2;
				depth /= 2;
				++uiNumMipLevels;
			}
			return uiNumMipLevels;
		}

		else
			return iMipMapSetting;
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

	void Texture::ClearToZero(std::uint32_t _mipLevel)
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

	void Texture::ReadImage(unsigned int _mipLevel, TextureReadFormat _format, TextureReadType _type, unsigned int _bufferSize, void* _buffer)
	{
		GLHELPER_ASSERT(m_numMipLevels > _mipLevel, "Miplevel " + std::to_string(_mipLevel) + " not available, texture has only " + std::to_string(m_numMipLevels) + " levels!");
		GL_CALL(glGetTextureImage, m_textureHandle, _mipLevel, static_cast<GLenum>(_format), static_cast<GLenum>(_type), _bufferSize, _buffer);
	}
}