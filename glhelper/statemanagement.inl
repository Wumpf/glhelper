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
