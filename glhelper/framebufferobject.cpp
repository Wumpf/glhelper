#include "framebufferobject.hpp"
#include "texture2d.hpp"

namespace gl
{
	FramebufferId FramebufferObject::s_BoundFrameBuffer = 0;
	//FramebufferObject* FramebufferObject::s_BoundFrameBufferRead = NULL;

	FramebufferObject::FramebufferObject(Attachment colorAttachments, Attachment depthStencil, bool depthWithStencil) :
		FramebufferObject({ colorAttachments }, depthStencil, depthWithStencil)
	{}

	FramebufferObject::FramebufferObject(std::initializer_list<Attachment> colorAttachments, Attachment depthStencil, bool depthWithStencil) :
		m_depthStencil(depthStencil)
	{
		GL_CALL(glCreateFramebuffers, 1, &m_framebuffer);

		if (depthStencil.pTexture)
		{
			GLint attachment = depthWithStencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;

			// Sufficient for MSAA?
			if (depthStencil.layer > 0)
				GL_CALL(glNamedFramebufferTextureLayer, m_framebuffer, attachment, depthStencil.pTexture->GetInternHandle(), depthStencil.mipLevel, depthStencil.layer);
			else
				GL_CALL(glNamedFramebufferTexture, m_framebuffer, attachment, depthStencil.pTexture->GetInternHandle(), depthStencil.mipLevel);
		}


		for (auto it = colorAttachments.begin(); it != colorAttachments.end(); ++it)
		{
			GLHELPER_ASSERT(it->pTexture, "FBO Color attachment texture is NULL!");
			GLint attachment = static_cast<GLint>(GL_COLOR_ATTACHMENT0 + m_colorAttachments.size());
			if (it->layer > 0)
				GL_CALL(glNamedFramebufferTextureLayer, m_framebuffer, attachment, it->pTexture->GetInternHandle(), it->mipLevel, it->layer);
			else
				GL_CALL(glNamedFramebufferTexture, m_framebuffer, attachment, it->pTexture->GetInternHandle(), it->mipLevel);

			m_colorAttachments.push_back(*it);
		}

		// setup draw buffers
		std::unique_ptr<GLuint[]> drawBuffers(new GLuint[m_colorAttachments.size()]);
		for (GLuint i = 0; i < m_colorAttachments.size(); ++i)
			drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
		GL_CALL(glNamedFramebufferDrawBuffers, m_framebuffer, static_cast<GLsizei>(m_colorAttachments.size()), drawBuffers.get());

		// Set read buffer to first color target.
		GL_CALL(glNamedFramebufferReadBuffer, m_framebuffer, GL_COLOR_ATTACHMENT0);

		// Error checking
		GLHELPER_ASSERT(m_depthStencil.pTexture != NULL || m_colorAttachments.size() > 0, "You cannot create empty FBOs! Need at least a depth/stencil buffer or a color attachment.");
		GLenum framebufferStatus = GL_RET_CALL(glCheckNamedFramebufferStatus, m_framebuffer, GL_FRAMEBUFFER);
		GLHELPER_ASSERT(framebufferStatus == GL_FRAMEBUFFER_COMPLETE, "Frame buffer creation failed! Error code: " + std::to_string(framebufferStatus));
	}

	FramebufferObject::FramebufferObject(FramebufferObject&& _moved) :
		m_framebuffer(_moved.m_framebuffer),
		m_depthStencil(_moved.m_depthStencil),
		m_colorAttachments(std::move(_moved.m_colorAttachments))
	{
		_moved.s_BoundFrameBuffer = 0;
	}

	FramebufferObject::~FramebufferObject()
	{
		if(m_framebuffer != 0)
		{
			if(s_BoundFrameBuffer == m_framebuffer)
				s_BoundFrameBuffer = 0;
			GL_CALL(glDeleteFramebuffers, 1, &m_framebuffer);
		}
	}

	void FramebufferObject::Bind(bool autoViewportSet)
	{
		if (s_BoundFrameBuffer != m_framebuffer)
		{
			GL_CALL(glBindFramebuffer, GL_DRAW_FRAMEBUFFER, m_framebuffer);
			s_BoundFrameBuffer = m_framebuffer;

			if (autoViewportSet)
			{
				Attachment* pSizeSource = NULL;
				if (m_depthStencil.pTexture)
					pSizeSource = &m_depthStencil;
				else
					pSizeSource = &m_colorAttachments[0];

				// Due to creation GLHELPER_ASSERTs pSizeSource should be now non zero!
				auto width = pSizeSource->pTexture->GetWidth();
				auto height = pSizeSource->pTexture->GetHeight();
				for (unsigned int mipLevel = 0; mipLevel < pSizeSource->mipLevel; ++mipLevel)
				{
					width /= 2;
					height /= 2;
				}
				glViewport(0, 0, width, height);
			}
		}
	}

	void FramebufferObject::BindBackBuffer()
	{
		if (s_BoundFrameBuffer != 0)
		{
			GL_CALL(glBindFramebuffer, GL_DRAW_FRAMEBUFFER, 0);
			s_BoundFrameBuffer = 0;
		}
	}


/*	void FramebufferObject::BlitTo(FramebufferObject* pDest, const ezRectU32& srcRect, const ezRectU32& dstRect, GLuint mask, GLuint filter)
	{
		glBlitNamedFramebuffer(m_framebuffer, pDest->m_framebuffer, srcRect.x, srcRect.y, srcRect.x + srcRect.width, srcRect.y + srcRect.height,
							dstRect.x, dstRect.y, dstRect.x + dstRect.width, dstRect.y + dstRect.height, mask, filter);
	} */
}