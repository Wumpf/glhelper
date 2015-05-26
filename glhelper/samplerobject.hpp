#pragma once

#include "gl.hpp"
#include "texture.hpp"
#include <unordered_map>

namespace gl
{
	/// \brief Easy to use OpenGL Sampler Object
	///
	/// Will store all runtime create sampler objects in a intern static list. List will be cleared on program shutdown or on explicit call.
	/// Also binds sampler only if not already bound.
	class SamplerObject
	{
	public:
		enum class Filter
		{
			NEAREST,
			LINEAR
		};
		enum class Border
		{
			REPEAT = GL_REPEAT,
			MIRROR = GL_MIRRORED_REPEAT,
			CLAMP = GL_CLAMP_TO_EDGE,
			BORDER = GL_CLAMP_TO_BORDER
		};

		enum class CompareMode
		{
			NONE,

			LESS_EQUAL = GL_LEQUAL,
			GREATER_EQUAL = GL_GEQUAL,
			LESS = GL_LESS,
			GREATER = GL_GREATER,
			EQUAL = GL_EQUAL,
			NOTEQUAL = GL_NOTEQUAL,
			ALWAYS = GL_ALWAYS,
			NEVER = GL_NEVER,
		};

		struct Desc
		{
			/// Constructs a descriptor with the same border handling for all dimensions.
			Desc(Filter minFilter, Filter magFilter, Filter mipFilter, Border borderHandling,
				unsigned int maxAnisotropy = 1, const gl::Vec4& borderColor = gl::Vec4(1.0f, 1.0f, 1.0f, 1.0f),
				CompareMode compareMode = CompareMode::NONE, float minLod = -1000.0f, float maxLod = 1000.0f);

			/// Constructs a descriptor with the different border handling for each dimension.
			Desc(Filter minFilter, Filter magFilter, Filter mipFilter, Border borderHandlingU, Border borderHandlingV, Border m_borderHandlingW,
				unsigned int maxAnisotropy = 1, const gl::Vec4& borderColor = gl::Vec4(1.0f, 1.0f, 1.0f, 1.0f),
				CompareMode compareMode = CompareMode::NONE, float minLod = -1000.0f, float maxLod = 1000.0f);

			bool operator == (const Desc& other) const { return memcmp(this, &other, sizeof(Desc)) == 0; }

			Filter minFilter;
			Filter magFilter;
			Filter mipFilter;
			Border borderHandlingU;
			Border borderHandlingV;
			Border borderHandlingW;
			unsigned int maxAnisotropy;
			gl::Vec4 borderColor;
			CompareMode compareMode;
			float minLod;
			float maxLod;


			struct GetHash
			{
				size_t operator()(const gl::SamplerObject::Desc& desc) const
				{
					// Super simple sdbm hash function.
					size_t hash = 0;
					int c;
					const unsigned char* str = reinterpret_cast<const unsigned char*>(this);
					for (int i = 0; i < sizeof(*this), c = *str++; ++i)
						hash = c + (hash << 6) + (hash << 16) - hash;
					return hash;
				}
			};
		};

		/// Will create or return a existing SamplerObject.
		static const SamplerObject& GetSamplerObject(const Desc& desc);

		/// Binds sampler to given texture stage if not already bound.
		///
		/// \attention Deleting a bound resource may cause unexpected errors. 
		///		The user is responsible for manual unbinding it (or overwriting the previous binding) before deleting.
		void BindSampler(GLuint _textureStage) const;

		/// Resets sampler binding for the given texture stage to zero.
		static void ResetBinding(GLuint _textureStage);

		/// Removes all existing sampler objects.
		void DestroyAllCachedSamplerObjects() { s_existingSamplerObjects.clear(); }

		/// Returns intern OpenGL sampler handle.
		SamplerId GetInternHandle() const { return m_samplerId; }

		/// Move constructor. Needed for intern hashmap.
		///
		/// A single redundant bind may happen if the sampler _cpy was bound to a slot, since this pointer will not replaced with the new sampler.
		SamplerObject(SamplerObject&& _cpy);
		~SamplerObject();

	private:
		SamplerObject(const Desc& samplerDesc);

		static const SamplerObject* s_samplerBindings[Texture::s_numTextureBindings];
		static std::unordered_map<Desc, SamplerObject, Desc::GetHash> s_existingSamplerObjects;

		SamplerId m_samplerId;
	};
};
