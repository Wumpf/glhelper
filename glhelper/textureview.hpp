#pragma once

#include "texture.hpp"

namespace gl
{
	/// Wrapper for a texture views.
	///
	/// TextureView are aliases of another's texture data store.
	/// See http://docs.gl/gl4/glTextureView
	class TextureView : public Texture
	{
	public:
		TextureView(const TextureView&) = delete;
		void operator = (const TextureView&) = delete;
		void operator = (TextureView&&) = delete;

		/// New texture view from existing texture.	
		TextureView(Texture& _originalTexture, TextureTarget _newTarget, TextureFormat _newFormat, 
					GLsizei _minMipLevel = 0, GLsizei _numMipLevels = std::numeric_limits<GLsizei>::max(),
					GLsizei _minLayer = 0, GLsizei _numLayers = 1);
		virtual ~TextureView();

		virtual GLenum GetOpenGLTextureType() const { return static_cast<GLenum>(m_newTarget); }

		const Texture& GetOriginalTexture() const { return m_originalTexture; }
		Texture& GetOriginalTexture() { return m_originalTexture; }

	private:
		Texture& m_originalTexture;
		TextureTarget m_newTarget;
	};
}
