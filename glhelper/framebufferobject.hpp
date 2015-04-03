#pragma once

#include "gl.hpp"
#include "shaderdatametainfo.hpp"


namespace gl
{
	class Texture;

	/// Abstraction for rendertarget objects
	/// \remarks Not yet supported:
	/// - Bind cubemap faces
	/// - Some MSAA functionality?
	/// - Blit multiple Targets at once
	/// - Clear multiple Targets at once
	/// - ...
	class FramebufferObject
	{
	public:
		FramebufferObject(const FramebufferObject&) = delete;
		void operator = (const FramebufferObject&) = delete;
		void operator = (FramebufferObject&&) = delete;

		struct Attachment
		{
			Attachment(Texture* pTexture, std::uint32_t mipLevel = 0, std::uint32_t layer = 0) :
				pTexture(pTexture), mipLevel(mipLevel), layer(layer) {}

			Texture* pTexture;
			std::uint32_t mipLevel;
			std::uint32_t layer;
		};

		FramebufferObject(Attachment _colorAttachment, Attachment _depthStencil = Attachment(NULL), bool _depthWithStencil = false);
		FramebufferObject(std::initializer_list<Attachment> _colorAttachments, Attachment _depthStencil = Attachment(NULL), bool _depthWithStencil = false);
		
		FramebufferObject(FramebufferObject&& _moved);

		~FramebufferObject();


		/// Binds the framebuffer object (GL_DRAW_FRAMEBUFFER).
		/// Has no effect if this FBO is already bound.
		/// \param autoViewportSet  If true an appropriate viewport will be set.
		void Bind(bool autoViewportSet);
		/// Resets the binding to zero (GL_DRAW_FRAMEBUFFER).
		/// Has no effect if backbuffer is already bound.
		/// You'll have to the the viewport on your own! 
		static void BindBackBuffer();

		/// Blits this Framebuffer to another one. Afterwards the dest buffer is set for drawing and this buffer is set for reading!
		/// \param pDest   Backbuffer if NULL
		/// \attention Will not change the viewport.
		//   void BlitTo(FramebufferObject* pDest, const ezRectU32& srcRect, const ezRectU32& dstRect, GLuint mask = GL_COLOR_BUFFER_BIT, GLuint filter = GL_NEAREST);


		/// Returns intern OpenGL framebuffer handle.
		FramebufferId GetInternHandle() const { return m_framebuffer; }


		const std::vector<Attachment>& GetColorAttachments() const { return m_colorAttachments; }
		const Attachment& GetDepthStencilAttachment() { return m_depthStencil; }

	private:
		/// Currently bound draw framebuffer object
		static FramebufferId s_BoundFrameBuffer;

		FramebufferId m_framebuffer;

		Attachment m_depthStencil;
		std::vector<Attachment> m_colorAttachments;
	};
}

