#pragma once

#include <unordered_set>
#include <unordered_map>

#include "gl.hpp"
#include "shaderdatametainfo.hpp"

namespace gl
{
	class UniformBufferView;
	class ShaderStorageBufferView;

	/// Easy to use wrapper for OpenGL shader.
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
		/// \param shaderName   Name mainly used for debugging and identification.
		ShaderObject(const std::string& _shaderName);
		ShaderObject(ShaderObject&& _moved);
		~ShaderObject();

		const std::string& GetName() const { return m_name; }

		Result AddShaderFromFile(ShaderType type, const std::string& sFilename, const std::string& prefixCode = "");
		Result AddShaderFromSource(ShaderType type, const std::string& pSourceCode, const std::string& sOriginName);
		Result CreateProgram();

		/// Returns raw gl program identifier (you know what you're doing, right?)
		GLuint GetProgram() const;

		/// Makes program active
		/// You can only activate one program at a time
		void Activate() const;


		// Manipulation of global uniforms
		// Make sure that the ShaderObject is already activated
		// (Setting of ordinary global uniform variables is explicitly not supported! You can still handle this yourself using the available Meta-Info)

		/// Binds an ubo by name
		Result BindUBO(UniformBufferView& ubo, const std::string& sUBOName);
		/// Binds an ubo by its intern buffer name
		Result BindUBO(UniformBufferView& ubo);

		/// Binds a shader storage buffer by name
		Result BindSSBO(ShaderStorageBufferView& _ssbo, const std::string& _SSBOName);
		/// Binds a shader storage buffer by its intern buffer name
		Result BindSSBO(ShaderStorageBufferView& _ssbo);


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

		const GlobalUniformInfos& GetGlobalUniformInfo() const { return m_globalUniformInfo; }

		/// Returns infos about used uniform buffer definitions
		/// \remarks Deliberately not const so user can use operator[] on the map
		UniformBlockInfos& GetUniformBufferInfo()    { return m_uniformBlockInfos; }
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
		void ShaderFileChangeHandler(const std::string& _changedShaderFile);

		/// Gets a list of all associated shader files (including resolved includes) and their usage.
		const std::unordered_map<std::string, ShaderType>& GetShaderFilenames() { return m_filesPerShaderType; }

	private:
		/// Print information about the compiling step
		void PrintShaderInfoLog(ShaderId shader, const std::string& sShaderName);
		/// Print information about the linking step
		void PrintProgramInfoLog(ProgramId program);

		/// Reads shader source code from file and performs parsing of #include directives
		/// \param fileIndex	This will used as second parameter for each #line macro. It is a kind of file identifier.
		/// \remarks Uses a lot of potentially slow string operations.
		static std::string ReadShaderFromFile(const std::string& shaderFilename, const std::string& prefixCode,
												unsigned int fileIndex, std::unordered_set<std::string>& _beforeIncludedFiles, std::unordered_set<std::string>& _allReadFiles);



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
			ShaderId  shaderObject;
			std::string  sOrigin;
			bool      loaded;
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