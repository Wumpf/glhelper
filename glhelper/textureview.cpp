#include "textureview.hpp"

namespace gl
{
	TextureView::TextureView(Texture& _originalTexture, TextureTarget _newTarget, TextureFormat _newFormat,
							GLsizei _minMipLevel, GLsizei _numMipLevels, GLsizei _minLayer, GLsizei _numLayers) :
		Texture(_originalTexture.GetWidth(), _originalTexture.GetHeight(), _originalTexture.GetDepth(), _newFormat, _originalTexture.GetNumMipLevels(), _originalTexture.GetNumMSAASamples()),
		m_originalTexture(_originalTexture),
		m_newTarget(_newTarget)
	{
		// TODO: Validation checks.

		GL_CALL(glGenTextures, 1, &m_textureHandle);
		GL_CALL(glTextureView, m_textureHandle, static_cast<GLenum>(_newTarget), _originalTexture.GetInternHandle(), gl::TextureFormatToGLSizedInternal[static_cast<unsigned int>(_newFormat)],
				_minLayer, _numMipLevels, _minLayer, _numLayers);
	}

	TextureView::~TextureView()
	{}
}