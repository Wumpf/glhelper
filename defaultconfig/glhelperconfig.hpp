// GLHelper is meant as "copy and paste"  helper class collection. As such you are encouraged to integrate it seamlessly.
// This files contains several type and function definitions that you usually want to alter for a better integration into your application.

#include <string>
#include <cassert>
#include <iostream>
#include <limits>  // for std::numeric_limits in textureview.hpp
#include <string.h> // for memcmp in samplerobject.h
#include <vector> // for std::vector in shaderobject.cpp
#include <algorithm> // for std::count in shaderobject.cpp

// General settings.

// If activated, Texture2D has a FromFile method which uses stbi to load images and create mipmaps.
//#define TEXTURE2D_FROMFILE_STBI

// Activates output of shader compile logs to log.
#define SHADER_COMPILE_LOGS



// Assert
#ifdef _DEBUG
#define GLHELPER_ASSERT(condition, message) do { \
	if(!(condition)) std::cerr << message; \
	assert(condition); } while(false)
#else
#define GLHELPER_ASSERT(condition, string) do { } while(false)
#endif



// Logging
#define GLHELPER_LOG_ERROR(message)		do { std::cerr << "Error: " << message << " (" << __FILE__ << ", (" << __LINE__ << ")" << std::endl; } while(false)
#define GLHELPER_LOG_WARNING(message)	do { std::cerr << "Warning: " << message << " (" << __FILE__ << ", (" << __LINE__ << ")" << std::endl; } while(false)
#define GLHELPER_LOG_INFO(message)		do { std::clog << "Info: " << message << " (" << __FILE__ << ", (" << __LINE__ << ")" << std::endl; } while(false)



// Vector & Matrix types.
#include <cstdint>
namespace gl
{
	namespace Details
	{
		template<typename T>
		struct DefaultVec2
		{
			DefaultVec2(T x, T y) : x(x), y(y) {}
			T x, y;
		};
		template<typename T>
		struct DefaultVec3
		{
			DefaultVec3(T x, T y, T z) : x(x), y(y), z(z) {}
			T x, y, z;
		};
		template<typename T>
		struct DefaultVec4
		{
			DefaultVec4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
			T x, y, z, w;
		};
		template<typename T>
		struct DefaultMat3x3
		{
			T m[3][3];
		};
		template<typename T>
		struct DefaultMat4x4
		{
			T m[4][4];
		};
	};

	typedef Details::DefaultVec2<float> Vec2;
	typedef Details::DefaultVec3<float> Vec3;
	typedef Details::DefaultVec4<float> Vec4;

	typedef Details::DefaultVec2<std::int32_t> IVec2;
	typedef Details::DefaultVec3<std::int32_t> IVec3;
	typedef Details::DefaultVec4<std::int32_t> IVec4;

	typedef Details::DefaultVec2<std::uint32_t> UVec2;
	typedef Details::DefaultVec3<std::uint32_t> UVec3;
	typedef Details::DefaultVec4<std::uint32_t> UVec4;

	typedef Details::DefaultMat3x3<float> Mat3;
	typedef Details::DefaultMat4x4<float> Mat4;
};



// OpenGL header.

#include <GL/glew.h>