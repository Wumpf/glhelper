inline void Enable(Cap _cap, bool _force)
{
	if (_force || Details::CapStates[static_cast<unsigned int>(_cap)] != CapState::ENABLED)
	{
		GL_CALL(glEnable, Details::CapStateToGLCap[static_cast<unsigned int>(_cap)]);
		Details::CapStates[static_cast<unsigned int>(_cap)] = CapState::ENABLED;
	}
}

inline void Disable(Cap _cap, bool _force)
{
	if (_force || Details::CapStates[static_cast<unsigned int>(_cap)] != CapState::DISABLED)
	{
		GL_CALL(glDisable, Details::CapStateToGLCap[static_cast<unsigned int>(_cap)]);
		Details::CapStates[static_cast<unsigned int>(_cap)] = CapState::DISABLED;
	}
}

inline void SetDepthWrite(bool _writeEnabled, bool _force)
{
	if (Details::DepthWriteEnabled != _writeEnabled || _force)
	{
		GL_CALL(glDepthMask, _writeEnabled ? GL_TRUE : GL_FALSE);
		Details::DepthWriteEnabled = _writeEnabled;
	}
}

inline bool GetDepthWrite()
{
	return Details::DepthWriteEnabled;
}

inline void SetDepthFunc(DepthFunc _depthCompFunc, bool _force)
{
	if (_depthCompFunc != Details::DepthComparisionFunc || _force)
	{
		GL_CALL(glDepthFunc, _depthCompFunc);
		Details::DepthComparisionFunc = _depthCompFunc;
	}
}

inline DepthFunc GetDepthFunc()
{
	return Details::DepthComparisionFunc;
}
