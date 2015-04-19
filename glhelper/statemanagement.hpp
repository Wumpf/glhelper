#pragma once

#include "gl.hpp"

/// \file statemanagement.hpp
/// Contains wrapper for various global state operations.
/// Motivation for this wrapper is first redundant state check and second typed enums for a better overview over available settings (and avoidance of typos concerning GL's macro values)

namespace gl
{
	/// List of all possible boolean capabilities
	/// 
	/// Docs are copied from http://docs.gl/gl4/glEnable.
	enum class Cap
	{
		/// \details If enabled, blend the computed fragment color values with the values in the color buffers. See glBlendFunc.
		BLEND,
		/// \details If enabled, clip geometry against user-defined half space i.
		CLIP_DISTANCE0,
		CLIP_DISTANCE1,
		CLIP_DISTANCE2,
		CLIP_DISTANCE3,
		CLIP_DISTANCE4,
		CLIP_DISTANCE5,
		/// \details If enabled, apply the currently selected logical operation to the computed fragment color and color buffer values. See glLogicOp.
		COLOR_LOGIC_OP,
		/// \details If enabled, cull polygons based on their winding in window coordinates. See glCullFace.
		CULL_FACE,
		/// \details If enabled, debug messages are produced by a debug context. When disabled, the debug message log is silenced. Note that in a non-debug context, very few, if any messages might be produced, even when GL_DEBUG_OUTPUT is enabled.
		DEBUG_OUTPUT,
		/// \details If enabled, debug messages are produced synchronously by a debug context. If disabled, debug messages may be produced asynchronously. In particular, they may be delayed relative to the execution of GL commands, and the debug callback function may be called from a thread other than that in which the commands are executed. See glDebugMessageCallback.
		DEBUG_OUTPUT_SYNCHRONOUS,
		/// \details If enabled, the −wc<=zc<=wc plane equation is ignored by view volume clipping (effectively, there is no near or far plane clipping). See glDepthRange.
		DEPTH_CLAMP,
		/// \details If enabled, do depth comparisons and update the depth buffer. Note that even if the depth buffer exists and the depth mask is non-zero, the depth buffer is not updated if the depth test is disabled. See glDepthFunc and glDepthRange.
		DEPTH_TEST,
		/// \details If enabled, dither color components or indices before they are written to the color buffer.
		DITHER,
		/// \details If enabled and the value of GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING for the framebuffer attachment corresponding to the destination buffer is GL_SRGB, the R, G, and B destination color values (after conversion from fixed-point to floating-point) are considered to be encoded for the sRGB color space and hence are linearized prior to their use in blending.
		FRAMEBUFFER_SRGB,
		/// \details If enabled, draw lines with correct filtering. Otherwise, draw aliased lines. See glLineWidth.
		LINE_SMOOTH,
		/// \details If enabled, use multiple fragment samples in computing the final color of a pixel. See glSampleCoverage.
		MULTISAMPLE,
		/// \details If enabled, and if the polygon is rendered in GL_FILL mode, an offset is added to depth values of a polygon's fragments before the depth comparison is performed. See glPolygonOffset.
		POLYGON_OFFSET_FILL,
		/// \details If enabled, and if the polygon is rendered in GL_LINE mode, an offset is added to depth values of a polygon's fragments before the depth comparison is performed. See glPolygonOffset.
		POLYGON_OFFSET_LINE,
		/// \details If enabled, an offset is added to depth values of a polygon's fragments before the depth comparison is performed, if the polygon is rendered in GL_POINT mode. See glPolygonOffset.
		POLYGON_OFFSET_POINT,
		/// \details If enabled, draw polygons with proper filtering. Otherwise, draw aliased polygons. For correct antialiased polygons, an alpha buffer is needed and the polygons must be sorted front to back.
		POLYGON_SMOOTH,
		/// \details Enables primitive restarting. If enabled, any one of the draw commands which transfers a set of generic attribute array elements to the GL will restart the primitive when the index of the vertex is equal to the primitive restart index. See glPrimitiveRestartIndex.
		PRIMITIVE_RESTART,
		/// \details Enables primitive restarting with a fixed index. If enabled, any one of the draw commands which transfers a set of generic attribute array elements to the GL will restart the primitive when the index of the vertex is equal to the fixed primitive index for the specified index type. The fixed index is equal to 2n−1 where n is equal to 8 for GL_UNSIGNED_BYTE, 16 for GL_UNSIGNED_SHORT and 32 for GL_UNSIGNED_INT.
		PRIMITIVE_RESTART_FIXED_INDEX,
		/// \details If enabled, primitives are discarded after the optional transform feedback stage, but before rasterization. Furthermore, when enabled, glClear, glClearBufferData, glClearBufferSubData, glClearTexImage, and glClearTexSubImage are ignored.
		RASTERIZER_DISCARD,
		/// \details If enabled, compute a temporary coverage value where each bit is determined by the alpha value at the corresponding sample location. The temporary coverage value is then ANDed with the fragment coverage value.
		SAMPLE_ALPHA_TO_COVERAGE,
		/// \details If enabled, each sample alpha value is replaced by the maximum representable alpha value.
		SAMPLE_ALPHA_TO_ONE,
		/// \details If enabled, the fragment's coverage is ANDed with the temporary coverage value. If GL_SAMPLE_COVERAGE_INVERT is set to GL_TRUE, invert the coverage value. See glSampleCoverage.
		SAMPLE_COVERAGE,
		/// \details If enabled, the active fragment shader is run once for each covered sample, or at fraction of this rate as determined by the current value of GL_MIN_SAMPLE_SHADING_VALUE. See glMinSampleShading.
		SAMPLE_SHADING,
		/// \details If enabled, the sample coverage mask generated for a fragment during rasterization will be ANDed with the value of GL_SAMPLE_MASK_VALUE before shading occurs. See glSampleMaski.
		SAMPLE_MASK,
		/// \details If enabled, discard fragments that are outside the scissor rectangle. See glScissor.
		SCISSOR_TEST,
		/// \details If enabled, do stencil testing and update the stencil buffer. See glStencilFunc and glStencilOp.
		STENCIL_TEST,
		/// \details If enabled, cubemap textures are sampled such that when linearly sampling from the border between two adjacent faces, texels from both faces are used to generate the final sample value. When disabled, texels from only a single face are used to construct the final sample value.
		TEXTURE_CUBE_MAP_SEAMLESS,
		/// \details If enabled and a vertex or geometry shader is active, then the derived point size is taken from the (potentially clipped) shader builtin gl_PointSize and clamped to the implementation-dependent point size range.
		PROGRAM_POINT_SIZE,

		NUM_CAPS
	};


	/// Possible states for gl::Disable/Enable.
	enum class CapState : char
	{
		UNKOWN = -1,
		DISABLED = 0,
		ENABLED = 1
	};

	namespace Details
	{
		/// http://delphigl.de/glcapsviewer/gl_stats_caps_single.php?listreportsbycap=GL_MAX_VIEWPORTS
		static const unsigned int MaxExpectedViewports = 16;

		/// http://delphigl.de/glcapsviewer/gl_stats_caps_single.php?listreportsbycap=GL_MAX_DRAW_BUFFERS
		static const unsigned int MaxExpectedDrawbuffers = 8;

		extern CapState BlendStatePerViewPort[MaxExpectedViewports];
		extern CapState ScissorTestPerViewPort[MaxExpectedDrawbuffers];
		extern CapState CapStates[static_cast<unsigned int>(Cap::NUM_CAPS)];

		extern GLenum CapStateToGLCap[static_cast<unsigned int>(Cap::NUM_CAPS)];
	};

	/// Enables a given state.
	///
	/// Calls glEnable if internal state is not ENABLED or force is true.
	inline void Enable(Cap _cap, bool _force = false);

	/// Disables a given state.
	///
	/// Calls glDisable if internal state is not DISABLED or force is true.
	inline void Disable(Cap _cap, bool _force = false);


	/// Enables a given state with index. Index is ignored for states other than BLEND and SCISSOR_TEST.
	///
	/// See https://www.opengl.org/wiki/GLAPI/glEnable#Indexed_Capabilities.
	/// Calls glEnablei if internal state is not ENABLED or force is true.
	/// Sets the non-indexed internal cap state to CapState::UNKOWN.
	void Enable(Cap _cap, GLuint _index, bool _force = false);

	/// Disables a given state. Index is ignored for states other than BLEND and SCISSOR_TEST.
	///
	/// See https://www.opengl.org/wiki/GLAPI/glEnable#Indexed_Capabilities.
	/// Calls glDisablei if internal state is not DISABLED or force is true.
	/// Sets the non-indexed internal cap state to CapState::UNKOWN.
	void Disable(Cap _cap, GLuint _index, bool _force = false);


	/// Returns value of cap from internal state table.
	inline CapState GetCapState(Cap _cap) { return Details::CapStates[static_cast<unsigned int>(_cap)]; }


	/// Resets the entire internal state table with result from glIsEnable on each state.
	/// 
	/// \attention This function is rather slow. Consider force flags or ResetBooleanStateTable_Unkown.
	/// \see ResetBooleanStateTable_Unkown, Enable, Disable
	void ResetBooleanCapStateTable_Get();

	/// Resets the internal state table to Details::EnableState::UNKOWN
	///
	/// For states which are set to unknown, no redundant change check will be performed. After a call to Enable/Disable a state is no longer unknown.
	/// \see ResetBooleanStateTable_Get, Enable, Disable
	void ResetBooleanCapStateTable_Unkown();


	// --------------------------------------------------------------------------------------------------------------------------
	// Depth
	// --------------------------------------------------------------------------------------------------------------------------

	/// Possible states for SetDepthFunc http://docs.gl/gl4/glDepthFunc
	enum DepthFunc
	{
		NEVER = GL_NEVER,		///< Never passes.
		LESS = GL_LESS,			///< Passes if the incoming depth value is less than the stored depth value.
		EQUAL = GL_EQUAL,		///< Passes if the incoming depth value is equal to the stored depth value.
		LEQUAL = GL_LEQUAL,		///< Passes if the incoming depth value is less than or equal to the stored depth value.
		GREATER = GL_GREATER,	///< Passes if the incoming depth value is greater than the stored depth value.
		NOTEQUAL = GL_NOTEQUAL,	///< Passes if the incoming depth value is not equal to the stored depth value.
		GEQUAL = GL_GEQUAL,		///< Passes if the incoming depth value is greater than or equal to the stored depth value.
		ALWAYS = GL_ALWAYS,		///< Always passes.
	};

	namespace Details
	{
		extern bool DepthWriteEnabled;
		extern DepthFunc DepthComparisionFunc;
	}

	/// Enable or disable writing into the depth buffer. (glDepthMask)
	///
	/// Reading from depth buffer is a cap state, since it is set by glEnable
	void SetDepthWrite(bool _writeEnabled, bool _force = false);

	/// Gets if writing to depth buffer is enabled. (glDepthMask)
	bool GetDepthWrite();

	/// Sets current depth buffer comparision function. (glDepthFunc)
	void SetDepthFunc(DepthFunc _depthCompFunc, bool _force = false);

	/// Gets current depth buffer comparision function.
	DepthFunc GetDepthFunc();

	// --------------------------------------------------------------------------------------------------------------------------
	// Stencil
	// --------------------------------------------------------------------------------------------------------------------------

	// todo ...


	// --------------------------------------------------------------------------------------------------------------------------
	// Blending
	// --------------------------------------------------------------------------------------------------------------------------

	// todo ...


	// --------------------------------------------------------------------------------------------------------------------------
	// Viewport
	// --------------------------------------------------------------------------------------------------------------------------

	// todo ...


	// --------------------------------------------------------------------------------------------------------------------------
	// Misc
	// --------------------------------------------------------------------------------------------------------------------------

	// todo ...
	#include "statemanagement.inl"
}

