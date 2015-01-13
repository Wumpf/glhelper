#pragma once

#include "gl.hpp"
#include "textureformats.hpp"
#include <cinttypes>

namespace gl
{
	class Texture
	{
	public:
		Texture(const Texture&) = delete;
		void operator = (const Texture&) = delete;
		void operator = (Texture&&) = delete;

		Texture(std::uint32_t _width, std::uint32_t _height, std::uint32_t _depth, TextureFormat _format, 
				 std::int32_t _numMipLevels, std::uint32_t _numMSAASamples = 0);
		Texture(Texture&& _moved);
		virtual ~Texture();

		/// Binds texture to the given slot.
		void Bind(GLuint _slotIndex);

        /// Unbinds texture.
        static void ResetBinding(GLuint _slotIndex);

		/// Different possibilities to access an image.
		enum class ImageAccess
		{
			WRITE = GL_WRITE_ONLY,
			READ = GL_READ_ONLY,
			READ_WRITE = GL_READ_WRITE
		};

		/// Binds as image.
		///
		/// Without redundancy checking! Does not perform checks if texture format is valid (see http://docs.gl/gl4/glBindImageTexture)!
		void BindImage(GLuint _slotIndex, ImageAccess _access) { BindImage(_slotIndex, _access, m_format); }
		void BindImage(GLuint _slotIndex, ImageAccess _access, TextureFormat _format);

		/// Reads image via glGetImage (http://docs.gl/gl4/glGetTexImage)
		void ReadImage(unsigned int _mipLevel, TextureReadFormat _format, TextureReadType _type, unsigned int _bufferSize, void* _buffer);

		/// Unbinds image slot.
		static void ResetImageBinding(GLuint _slotIndex);

		/// Clears texture to zero using glClearTexImage (http://docs.gl/gl4/glClearTexImage)
		void ClearToZero(std::uint32_t _mipLevel = 0);

		/// Returns intern OpenGL texture handle.
		TextureId GetInternHandle() { return m_textureHandle; }

		std::uint32_t GetWidth() const           { return m_width; }
		std::uint32_t GetHeight() const          { return m_height; }
		std::uint32_t GetDepth() const           { return m_depth; }
		std::uint32_t GetNumMipLevels() const    { return m_numMipLevels; }
		std::uint32_t GetNumMSAASamples() const  { return m_numMSAASamples; }
		TextureFormat GetFormat() const          { return m_format; }

		virtual GLenum GetOpenGLTextureType() = 0;

		/// Max number of tracked texture bindings. Arbitrary number based on observation: http://delphigl.de/glcapsviewer/gl_stats_caps_single.php?listreportsbycap=GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS
		static const unsigned int s_numTextureBindings = 192;

	protected:
		friend class TextureBufferView;

		/// Binds given texture to a slot.
		///
		/// Does nothing if texture was already bound.
		/// \remarks Internally used for all textures and texturebuffer.
		/// Usually you should use TextureXD::Bind or TextureBuffer::Bind
		static void Bind(TextureId textureHandle, GLuint _slotIndex);

		/// Currently bound textures - number is arbitrary!
		/// Also used for texturebuffer since these bindings are the same for OpenGL.
		/// Not used for image binding
		static TextureId s_boundTextures[s_numTextureBindings];
		
		TextureId m_textureHandle;

		const std::uint32_t m_width;
		const std::uint32_t m_height;
		const std::uint32_t m_depth;

		const TextureFormat m_format;
		const std::uint32_t  m_numMipLevels;
		const std::uint32_t  m_numMSAASamples;

	private:
		static std::uint32_t ConvertMipMapSettingToActualCount(std::int32_t iMipMapSetting, std::uint32_t width, std::uint32_t height, std::uint32_t depth = 0);
	};

}
