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
		ScreenAlignedTriangle(const ScreenAlignedTriangle&) = delete;
		void operator = (const ScreenAlignedTriangle&) = delete;

		ScreenAlignedTriangle();
		~ScreenAlignedTriangle();

		void Draw() const;

	private:
		Buffer* m_vertexBuffer;
		VertexArrayObject* m_vertexArrayObject;
	};
}