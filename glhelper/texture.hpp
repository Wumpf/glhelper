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

		/// New abstract texture.
		///
		/// \param _numMipLevels
		///		Number of MipMap levels. If 0, the maximum possible number of mipmaps will be determined and used.
		Texture(GLsizei _width, GLsizei _height, GLsizei _depth, TextureFormat _format, GLsizei _numMipLevels, GLsizei _numMSAASamples = 0);
		Texture(Texture&& _moved);
		virtual ~Texture();

		/// Binds texture to the given slot.
		void Bind(GLuint _slotIndex) const;

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
		void BindImage(GLuint _slotIndex, ImageAccess _access) const { BindImage(_slotIndex, _access, m_format); }
		void BindImage(GLuint _slotIndex, ImageAccess _access, TextureFormat _format) const;

		/// Reads image via glGetImage (http://docs.gl/gl4/glGetTexImage)
		void ReadImage(GLsizei _mipLevel, TextureReadFormat _format, TextureReadType _type, GLsizei _bufferSize, void* _buffer) const;

		/// Unbinds image slot.
		static void ResetImageBinding(GLuint _slotIndex);

		/// Clears texture to zero using glClearTexImage (http://docs.gl/gl4/glClearTexImage)
		void ClearToZero(GLsizei _mipLevel = 0);

		/// Returns intern OpenGL texture handle.
		TextureId GetInternHandle() const { return m_textureHandle; }

		/// Returns texture width (for mipLevel==0).
		GLsizei GetWidth() const           { return m_width; }

		/// Returns texture height (for mipLevel==0).
		GLsizei GetHeight() const          { return m_height; }

		/// Returns total texture depth (for mipLevel==0)
		/// 1 for non 3D textures.
		GLsizei GetDepth() const           { return m_depth; }

		GLsizei GetNumMipLevels() const    { return m_numMipLevels; }
		GLsizei GetNumMSAASamples() const  { return m_numMSAASamples; }
		TextureFormat GetFormat() const    { return m_format; }

		virtual GLenum GetOpenGLTextureType() const = 0;

		/// Generates mipmaps via glGenerateMipmap (http://docs.gl/gl4/glGenerateMipmap)
		void GenMipMaps();

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

		const GLsizei m_width;
		const GLsizei m_height;
		const GLsizei m_depth;

		const TextureFormat m_format;
		const GLsizei m_numMipLevels;
		const GLsizei m_numMSAASamples;

	private:
		static GLsizei ConvertMipMapSettingToActualCount(GLsizei iMipMapSetting, GLsizei width, GLsizei height, GLsizei depth = 0);
	};

}
