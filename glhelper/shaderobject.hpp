#pragma once

#include <unordered_set>
#include <unordered_map>

#include "gl.hpp"
#include "shaderdatametainfo.hpp"

namespace gl
{
	class Buffer;

	/// Easy to use wrapper for OpenGL shader. Supports #include and various reflection options.
	class ShaderObject
	{
	public:
		typedef std::unordered_map<std::string, UniformVariableInfo> GlobalUniformInfos;
		typedef std::unordered_map<std::string, ShaderStorageBufferMetaInfo> ShaderStorageInfos;
		typedef std::unordered_map<std::string, UniformBufferMetaInfo> UniformBlockInfos;

		ShaderObject(const ShaderObject&) = delete;
		void operator = (const ShaderObject&) = delete;
		void operator = (ShaderObject&&) = delete;

		enum class ShaderType
		{
			VERTEX,
			FRAGMENT,
			EVALUATION,
			CONTROL,
			GEOMETRY,
			COMPUTE,

			NUM_SHADER_TYPES
		};

		/// Constructs ShaderObject
		/// 
		/// To add code use AddShaderFromFile/AddShaderFromSource and call CreateProgram if you are done.
		/// \param shaderName   Name mainly used for debugging and identification.
		ShaderObject(const std::string& _shaderName);
		ShaderObject(ShaderObject&& _moved);
		~ShaderObject();

		const std::string& GetName() const { return m_name; }

		/// Adds a shader file from a given source file.
		///
		/// Tries to resolve all #includes relative to the filename. You need to call CreateProgram in order to link the Shader.
		/// \param _type
		///   Type of the added shader.
		/// \param _filename
		///   Path to the shader file (text).
		/// \param _prefixCode
		///   Code inserted after the version tag.
		/// \see AddShaderFromSource, CreateProgram
		Result AddShaderFromFile(ShaderType _type, const std::string& _filename, const std::string& _prefixCode = "");

		/// Adds a shader file from raw source text.
		///
		/// Does not resolve any #includes! You need to call CreateProgram in order to link the Shader.
		/// \param _type
		///   Type of the added shader.
		/// \param _sourceCode
		///   GLSL shader code.
		/// \param _originName
		///   Origin name used for identifying.
		/// \see AddShaderFromSource, CreateProgram
		Result AddShaderFromSource(ShaderType _type, const std::string& _sourceCode, const std::string& _originName);

		/// Links all previously added shader to an OpenGL program.
		Result CreateProgram();

		/// Returns raw gl program identifier (you know what you're doing, right?)
		GLuint GetProgram() const;

		/// Makes program active.
		/// 
		/// You can only activate one program at a time. Checks for redundant state changes.
		void Activate() const;



		/// Binds an ubo by name.
		Result BindUBO(Buffer& _ubo, const std::string& _UBOName) const;

		/// Binds a shader storage buffer by name.
		Result BindSSBO(Buffer& _ssbo, const std::string& _SSBOName) const;

		/// The set of active user-defined inputs to the first shader stage in this program. 
		/// 
		/// If the first stage is a Vertex Shader, then this is the list of active attributes.
		/// If the program only contains a Compute Shader, then there are no inputs.
		GLint GetTotalProgramInputCount() const   { return m_totalProgramInputCount; }

		/// The set of active user-defined outputs from the final shader stage in this program.
		/// 
		/// If the final stage is a Fragment Shader, then this represents the fragment outputs that get written to individual color buffers.
		/// If the program only contains a Compute Shader, then there are no outputs.
		GLint GetTotalProgramOutputCount() const  { return m_totalProgramOutputCount; }



		/// Returns infos about global uniforms
		/// \remarks Deliberately not const so user can use operator[] on the map
		GlobalUniformInfos& GetGlobalUniformInfo()    { return m_globalUniformInfo; }
		/// \copydoc GetGlobalUniformInfo
		const GlobalUniformInfos& GetGlobalUniformInfo() const { return m_globalUniformInfo; }

		/// Returns infos about used uniform buffer definitions
		/// \remarks Deliberately not const so user can use operator[] on the map
		UniformBlockInfos& GetUniformBufferInfo()    { return m_uniformBlockInfos; }
		/// \copydoc GetUniformBufferInfo
		const UniformBlockInfos& GetUniformBufferInfo() const { return m_uniformBlockInfos; }

		/// Returns infos about used shader storage buffer definitions
		const ShaderStorageInfos& GetShaderStorageBufferInfo() const    { return m_shaderStorageInfos; }


		/// Returns a binary representation of the shader.
		///
		/// See http://docs.gl/gl4/glGetProgramBinary
		/// \attention It is driver specific!
		/// \param _binaryFormat
		///   A token that specifies the format of the binary data.
		std::vector<char> GetProgramBinary(GLenum& _binaryFormat);



		/// Call this function for hot reloading of a shader.
		/// If the given filename is not recognized, nothing will happen.
		/// If the file was loaded with a prefix code (see AddShaderFromFile) then this code will also be used again.
		void ShaderFileChangeHandler(const std::string& _changedShaderFile);

		/// Reload a program with new options.
		/// This call will reload all shader files attached to the program.
		Result SettingsChangeHandler(const std::string& _newPrefixCode);

		/// Gets a list of all associated shader files (including resolved includes) and their usage.
		const std::unordered_map<std::string, ShaderType>& GetShaderFilenames() { return m_filesPerShaderType; }

	private:
		/// Print information about the compiling step
		void PrintShaderInfoLog(ShaderId _shader, const std::string& _shaderName);
		/// Print information about the linking step
		void PrintProgramInfoLog(ProgramId _program);

		/// Reads shader source code from file and performs parsing of #include directives
		/// \param fileIndex	This will used as second parameter for each #line macro. It is a kind of file identifier.
		/// \remarks Uses a lot of potentially slow string operations.
		static std::string ReadShaderFromFile(const std::string& shaderFilename, const std::string& prefixCode,
												unsigned int fileIndex, std::unordered_set<std::string>& _beforeIncludedFiles, std::unordered_set<std::string>& _allReadFiles);

		/// Internal function called by AddShaderFromSource and AddShaderFromFile
		Result AddShader(ShaderType _type, const std::string& _sourceCode, const std::string& _originName, const std::string& _prefixCode);


		/// queries uniform informations from the program
		void QueryProgramInformations();

		/// Intern helper function for gather general BufferInformations
		template<typename BufferVariableType>
		void QueryBlockInformations(std::unordered_map<std::string, BufferInfo<BufferVariableType>>& BufferToFill, GLenum InterfaceName);


		/// Name for identifying at runtime
		const std::string m_name;

		// the program itself
		ProgramId m_program;
		bool m_containsAssembledProgram;

		/// currently active shader object
		/// As long as no user bypasses the Activate mechanism by calling glUseProgram, this pointer will always point the currently bound program.
		static const ShaderObject* s_currentlyActiveShaderObject;

		/// list of relevant files - if any of these changes a reload can be triggered via ShaderFileChangeHandler.
		std::unordered_map<std::string, ShaderType> m_filesPerShaderType;

		// underlying shaders
		struct Shader
		{
			ShaderId shaderObject;
			std::string origin;
			std::string prefixCode;
			bool loaded;
		};
		Shader m_shader[(unsigned int)ShaderType::NUM_SHADER_TYPES];

		// meta information
		GlobalUniformInfos m_globalUniformInfo;
		UniformBlockInfos  m_uniformBlockInfos;
		ShaderStorageInfos m_shaderStorageInfos;

		// misc
		GLint m_totalProgramInputCount;  ///< \see GetTotalProgramInputCount
		GLint m_totalProgramOutputCount; ///< \see GetTotalProgramOutputCount

		// currently missing meta information
		// - transform feedback buffer
		// - transform feedback varying
		// - subroutines
		// - atomic counter buffer
	};
}