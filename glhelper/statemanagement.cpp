#include "statemanagement.hpp"

namespace gl
{
	namespace Details
	{
		// Conversion table.
		GLenum CapStateToGLCap[static_cast<unsigned int>(Cap::NUM_CAPS)] = {
			GL_BLEND, GL_CLIP_DISTANCE0, GL_CLIP_DISTANCE1, GL_CLIP_DISTANCE2, GL_CLIP_DISTANCE3, GL_CLIP_DISTANCE4, GL_CLIP_DISTANCE5, GL_COLOR_LOGIC_OP, GL_CULL_FACE, GL_DEBUG_OUTPUT, GL_DEBUG_OUTPUT_SYNCHRONOUS, GL_DEPTH_CLAMP, GL_DEPTH_TEST, GL_DITHER, GL_FRAMEBUFFER_SRGB, GL_LINE_SMOOTH, GL_MULTISAMPLE, GL_POLYGON_OFFSET_FILL, GL_POLYGON_OFFSET_LINE, GL_POLYGON_OFFSET_POINT, GL_POLYGON_SMOOTH, GL_PRIMITIVE_RESTART, GL_PRIMITIVE_RESTART_FIXED_INDEX, GL_RASTERIZER_DISCARD, GL_SAMPLE_ALPHA_TO_COVERAGE, GL_SAMPLE_ALPHA_TO_ONE, GL_SAMPLE_COVERAGE, GL_SAMPLE_SHADING, GL_SAMPLE_MASK, GL_SCISSOR_TEST, GL_STENCIL_TEST, GL_TEXTURE_CUBE_MAP_SEAMLESS, GL_PROGRAM_POINT_SIZE,
		};

		// Initial states according to http://docs.gl/gl4/glEnable
		CapState CapStates[static_cast<unsigned int>(Cap::NUM_CAPS)]
		{
			CapState::DISABLED, // BLEND
			CapState::DISABLED, // CLIP_DISTANCE0
			CapState::DISABLED, // CLIP_DISTANCE1
			CapState::DISABLED, // CLIP_DISTANCE2
			CapState::DISABLED, // CLIP_DISTANCE3
			CapState::DISABLED, // CLIP_DISTANCE4
			CapState::DISABLED, // CLIP_DISTANCE5
			CapState::DISABLED, // COLOR_LOGIC_OP
			CapState::DISABLED, // CULL_FACE
			CapState::DISABLED, // DEBUG_OUTPUT
			CapState::DISABLED, // DEBUG_OUTPUT_SYNCHRONOUS
			CapState::DISABLED, // DEPTH_CLAMP
			CapState::DISABLED, // DEPTH_TEST
			CapState::ENABLED, // DITHER
			CapState::DISABLED, // FRAMEBUFFER_SRGB
			CapState::DISABLED, // LINE_SMOOTH
			CapState::ENABLED, // MULTISAMPLE
			CapState::DISABLED, // POLYGON_OFFSET_FILL
			CapState::DISABLED, // POLYGON_OFFSET_LINE
			CapState::DISABLED, // POLYGON_OFFSET_POINT
			CapState::DISABLED, // POLYGON_SMOOTH
			CapState::DISABLED, // PRIMITIVE_RESTART
			CapState::DISABLED, // PRIMITIVE_RESTART_FIXED_INDEX
			CapState::DISABLED, // RASTERIZER_DISCARD
			CapState::DISABLED, // SAMPLE_ALPHA_TO_COVERAGE
			CapState::DISABLED, // SAMPLE_ALPHA_TO_ONE
			CapState::DISABLED, // SAMPLE_COVERAGE
			CapState::DISABLED, // SAMPLE_SHADING
			CapState::DISABLED, // SAMPLE_MASK
			CapState::DISABLED, // SCISSOR_TEST
			CapState::DISABLED, // STENCIL_TEST
			CapState::DISABLED, // TEXTURE_CUBE_MAP_SEAMLESS
			CapState::DISABLED, // PROGRAM_POINT_SIZE
		};

		CapState BlendStatePerViewPort[Details::MaxExpectedViewports];
		CapState ScissorTestPerViewPort[Details::MaxExpectedDrawbuffers];

		void InitScissorBlendState()
		{
			for (unsigned int i = 0; i < Details::MaxExpectedViewports; ++i)
				BlendStatePerViewPort[i] = CapState::DISABLED;
			for (unsigned int i = 0; i < Details::MaxExpectedDrawbuffers; ++i)
				ScissorTestPerViewPort[i] = CapState::DISABLED;
		}
		struct InitScissorBlendStateOnStartup
		{
			InitScissorBlendStateOnStartup() { InitScissorBlendState(); }
		} initScissorBlendStateOnStartup;
	}

	void Enable(Cap _cap, GLuint _index, bool _force)
	{
		if (_cap == Cap::BLEND)
		{
			Assert(_index < Details::MaxExpectedViewports, "Enable index exceeds expected maximum!");

			if (_force || Details::CapStates[static_cast<unsigned int>(_cap)] == CapState::DISABLED ||
				(Details::CapStates[static_cast<unsigned int>(_cap)] == CapState::UNKOWN && Details::BlendStatePerViewPort[_index] != CapState::ENABLED))
			{
				GL_CALL(glEnablei, Details::CapStateToGLCap[static_cast<unsigned int>(_cap)], _index);
				Details::CapStates[static_cast<unsigned int>(_cap)] = CapState::UNKOWN;
				Details::BlendStatePerViewPort[_index] = CapState::ENABLED;
			}
		}
		else if (_cap == Cap::SCISSOR_TEST)
		{
			Assert(_index < Details::MaxExpectedDrawbuffers, "Enable index exceeds expected maximum!");

			if (_force || Details::CapStates[static_cast<unsigned int>(_cap)] == CapState::DISABLED ||
				(Details::CapStates[static_cast<unsigned int>(_cap)] == CapState::UNKOWN && Details::ScissorTestPerViewPort[_index] != CapState::ENABLED))
			{
				GL_CALL(glEnablei, Details::CapStateToGLCap[static_cast<unsigned int>(_cap)], _index);
				Details::CapStates[static_cast<unsigned int>(_cap)] = CapState::UNKOWN;
				Details::ScissorTestPerViewPort[_index] = CapState::ENABLED;
			}
		}
		else
		{
			Enable(_cap, _force);
		}
	}

	void Disable(Cap _cap, GLuint _index, bool _force)
	{
		if (_cap == Cap::BLEND)
		{
			Assert(_index < Details::MaxExpectedViewports, "Enable index exceeds expected maximum!");

			if (_force || Details::CapStates[static_cast<unsigned int>(_cap)] == CapState::ENABLED ||
				(Details::CapStates[static_cast<unsigned int>(_cap)] == CapState::UNKOWN && Details::BlendStatePerViewPort[_index] != CapState::DISABLED))
			{
				GL_CALL(glDisablei, Details::CapStateToGLCap[static_cast<unsigned int>(_cap)], _index);
				Details::CapStates[static_cast<unsigned int>(_cap)] = CapState::UNKOWN;
				Details::BlendStatePerViewPort[_index] = CapState::ENABLED;
			}
		}
		else if (_cap == Cap::SCISSOR_TEST)
		{
			Assert(_index < Details::MaxExpectedDrawbuffers, "Enable index exceeds expected maximum!");

			if (_force || Details::CapStates[static_cast<unsigned int>(_cap)] == CapState::ENABLED ||
				(Details::CapStates[static_cast<unsigned int>(_cap)] == CapState::UNKOWN && Details::ScissorTestPerViewPort[_index] != CapState::DISABLED))
			{
				GL_CALL(glDisablei, Details::CapStateToGLCap[static_cast<unsigned int>(_cap)], _index);
				Details::CapStates[static_cast<unsigned int>(_cap)] = CapState::UNKOWN;
				Details::ScissorTestPerViewPort[_index] = CapState::DISABLED;
			}
		}
		else
		{
			Enable(_cap, _force);
		}
	}

	void ResetBooleanStateTable_Get()
	{
		for (unsigned int i = 0; i < static_cast<unsigned int>(Cap::NUM_CAPS); ++i)
			Details::CapStates[i] = glIsEnabled(Details::CapStateToGLCap[i]) == GL_TRUE ? CapState::ENABLED : CapState::DISABLED;
	}

	void ResetBooleanStateTable_Unkown()
	{
		for (unsigned int i = 0; i < static_cast<unsigned int>(Cap::NUM_CAPS); ++i)
			Details::CapStates[i] = CapState::UNKOWN;
	}
}