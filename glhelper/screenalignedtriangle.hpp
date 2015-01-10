#pragma once

#include "gl.hpp"

namespace gl
{
	class Buffer;
	class VertexArrayObject;

	// class for rendering a screen aligned triangle
	class ScreenAlignedTriangle
	{
	public:
		ScreenAlignedTriangle();
		~ScreenAlignedTriangle();

		void Draw();

	private:
		Buffer* m_vertexBuffer;
		VertexArrayObject* m_vertexArrayObject;
	};
}