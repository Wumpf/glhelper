#include "samplerobject.hpp"

namespace gl
{
	std::unordered_map<SamplerObject::Desc, SamplerObject, SamplerObject::Desc::GetHash> SamplerObject::s_existingSamplerObjects;
	const SamplerObject* SamplerObject::s_samplerBindings[gl::Texture::s_numTextureBindings];

	SamplerObject::Desc::Desc(Filter minFilter, Filter magFilter, Filter mipFilter,
		Border borderHandling, unsigned int maxAnisotropy, const gl::Vec4& borderColor, CompareMode compareMode, 
		float minLod, float maxLod) :
		Desc(minFilter, magFilter, mipFilter, borderHandling, borderHandling, borderHandling, maxAnisotropy, borderColor, compareMode, minLod, maxLod)
	{}

	SamplerObject::Desc::Desc(Filter minFilter, Filter magFilter, Filter mipFilter,
		Border borderHandlingU, Border borderHandlingV, Border m_borderHandlingW,
		unsigned int maxAnisotropy, const gl::Vec4& borderColor, CompareMode compareMode, 
		float minLod, float maxLod) :
		minFilter(minFilter),
		magFilter(magFilter),
		mipFilter(mipFilter),
		borderHandlingU(borderHandlingU),
		borderHandlingV(borderHandlingV),
		borderHandlingW(m_borderHandlingW),
		maxAnisotropy(maxAnisotropy),
		borderColor(borderColor),
		compareMode(compareMode),
		minLod(minLod),
		maxLod(maxLod)
	{
	}

	SamplerObject::SamplerObject(SamplerObject&& cpy) :
		m_samplerId(cpy.m_samplerId)
	{
		cpy.m_samplerId = 0;
	}


	SamplerObject::SamplerObject(const Desc& desc)
	{
		GLHELPER_ASSERT(desc.maxAnisotropy > 0, "Anisotropy level of 0 is invalid! Must be between 1 and GPU's max.");

		GL_CALL(glGenSamplers, 1, &m_samplerId);
		GL_CALL(glSamplerParameteri, m_samplerId, GL_TEXTURE_WRAP_S, static_cast<GLint>(desc.borderHandlingU));
		GL_CALL(glSamplerParameteri, m_samplerId, GL_TEXTURE_WRAP_T, static_cast<GLint>(desc.borderHandlingV));
		GL_CALL(glSamplerParameteri, m_samplerId, GL_TEXTURE_WRAP_R, static_cast<GLint>(desc.borderHandlingW));

		GLint minFilterGl;
		if (desc.minFilter == Filter::NEAREST)
			minFilterGl = desc.mipFilter == Filter::NEAREST ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_LINEAR;
		else
			minFilterGl = desc.mipFilter == Filter::NEAREST ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR;

		GL_CALL(glSamplerParameteri, m_samplerId, GL_TEXTURE_MIN_FILTER, minFilterGl);
		GL_CALL(glSamplerParameteri, m_samplerId, GL_TEXTURE_MAG_FILTER, desc.magFilter == Filter::NEAREST ? GL_NEAREST : GL_LINEAR);

		GL_CALL(glSamplerParameterf, m_samplerId, GL_TEXTURE_MIN_LOD, desc.minLod);
		GL_CALL(glSamplerParameterf, m_samplerId, GL_TEXTURE_MAX_LOD, desc.maxLod);

		GL_CALL(glSamplerParameteri, m_samplerId, GL_TEXTURE_MAX_ANISOTROPY_EXT, desc.maxAnisotropy);

		GL_CALL(glSamplerParameterfv, m_samplerId, GL_TEXTURE_BORDER_COLOR, reinterpret_cast<const float*>(&desc.borderColor));

		if (desc.compareMode != CompareMode::NONE)
		{
			GL_CALL(glSamplerParameteri, m_samplerId, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			GL_CALL(glSamplerParameteri, m_samplerId, GL_TEXTURE_COMPARE_FUNC, static_cast<GLenum>(desc.compareMode));
		}
	}

	SamplerObject::~SamplerObject()
	{
		if (m_samplerId != 0)
		{
			// According to the specification it is not necessary to unbind the samplerobject. All associated sampler bindings reset themselves to zero.
			// http://docs.gl/gl4/glDeleteSamplers
			// However this means, that glhelper's saved bindings are wrong.
			// Iterating over all bindings is rather costly but reliable, easy and zero overhead for all other operations.
			for (unsigned int i = 0; i < Texture::s_numTextureBindings; ++i)
			{
				if (s_samplerBindings[i] == this)
					s_samplerBindings[i] = nullptr;
			}

			GL_CALL(glDeleteSamplers, 1, &m_samplerId);
		}
	}

	const SamplerObject& SamplerObject::GetSamplerObject(const Desc& desc)
	{
		auto it = s_existingSamplerObjects.find(desc);
		if (it == s_existingSamplerObjects.end())
		{
			auto newElem = s_existingSamplerObjects.emplace(desc, SamplerObject(desc));
			return newElem.first->second;
		}

		return it->second;
	}

	void SamplerObject::BindSampler(GLuint _textureStage) const
	{
		GLHELPER_ASSERT(_textureStage < sizeof(s_samplerBindings) / sizeof(SamplerObject*), "Can't bind sampler to slot " << _textureStage << " .Maximum number of slots is " << sizeof(s_samplerBindings) / sizeof(SamplerObject*));
		if (s_samplerBindings[_textureStage] != this)
		{
			GL_CALL(glBindSampler, _textureStage, m_samplerId);
			s_samplerBindings[_textureStage] = this;
		}
	}

	void SamplerObject::ResetBinding(GLuint _textureStage)
	{
		GLHELPER_ASSERT(_textureStage < sizeof(s_samplerBindings) / sizeof(SamplerObject*), "Can't bind sampler to slot " << _textureStage << " .Maximum number of slots is " << sizeof(s_samplerBindings) / sizeof(SamplerObject*));
		GL_CALL(glBindSampler, _textureStage, 0);
		s_samplerBindings[_textureStage] = nullptr;
	}
}