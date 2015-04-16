#include "shaderobject.hpp"
#include "utils/pathutils.hpp"

#include "buffer.hpp"

#include <iostream>
#include <fstream>
#include <memory>

namespace gl
{
	/// Global event for changed shader files.
	/// All Shader Objects will register upon this event. If any shader file is changed, just brodcast here!
	//ezEvent<const std::string&> ShaderObject::s_shaderFileChangedEvent;

	const ShaderObject* ShaderObject::s_currentlyActiveShaderObject = NULL;

	ShaderObject::ShaderObject(const std::string& _name) :
		m_name(_name),
		m_program(0),
		m_containsAssembledProgram(false)
	{
		for (Shader& shader : m_shader)
		{
			shader.shaderObject = 0;
			shader.sOrigin = "";
			shader.loaded = false;
		}
	}

	ShaderObject::ShaderObject(ShaderObject&& _moved) :
		m_name(std::move(_moved.m_name)),
		m_program(_moved.m_program),
		m_containsAssembledProgram(_moved.m_containsAssembledProgram),
		m_filesPerShaderType(std::move(_moved.m_filesPerShaderType)),

		m_globalUniformInfo(std::move(_moved.m_globalUniformInfo)),
		m_uniformBlockInfos(std::move(_moved.m_uniformBlockInfos)),
		m_shaderStorageInfos(std::move(_moved.m_shaderStorageInfos)),

		m_totalProgramInputCount(_moved.m_totalProgramInputCount),
		m_totalProgramOutputCount(_moved.m_totalProgramOutputCount)
	{
		for(unsigned int i = 0; i < static_cast<unsigned int>(ShaderType::NUM_SHADER_TYPES); ++i)
		{
			m_shader[i] = std::move(_moved.m_shader[i]);
			_moved.m_shader[i].shaderObject = 0;
		}
		_moved.m_program = 0;
	}

	ShaderObject::~ShaderObject()
	{
		if(m_program)
		{
			if(s_currentlyActiveShaderObject == this)
			{
				// Program must be detached to be able to delete it!
				// http://docs.gl/gl4/glDeleteShader
				GL_CALL(glUseProgram, 0);
				s_currentlyActiveShaderObject = NULL;
			}

			for(Shader& shader : m_shader)
			{
				if(shader.loaded)
					GL_CALL(glDeleteShader, shader.shaderObject);
			}

			if(m_containsAssembledProgram)
				GL_CALL(glDeleteProgram, m_program);
		}
	}

	Result ShaderObject::AddShaderFromFile(ShaderType type, const std::string& filename, const std::string& prefixCode)
	{
		// load new code
		std::unordered_set<std::string> includingFiles, allFiles;
		std::string sourceCode = ReadShaderFromFile(filename, prefixCode, 0, includingFiles, allFiles);
		if (sourceCode == "")
			return Result::FAILURE;

		Result result = AddShaderFromSource(type, sourceCode, filename);

		if (result != Result::FAILURE)
		{
			// memorize files
			for (auto it = allFiles.begin(); it != allFiles.end(); ++it)
				m_filesPerShaderType.emplace(*it, type);
		}

		return result;
	}

	/// \param _beforeIncludedFiles
	///		Does not include THIS file, but all files before.
	std::string ShaderObject::ReadShaderFromFile(const std::string& shaderFilename, const std::string& prefixCode,
												 unsigned int fileIndex, std::unordered_set<std::string>& _beforeIncludedFiles, std::unordered_set<std::string>& _allReadFiles)
	{
		// open file
		std::ifstream file(shaderFilename.c_str());
		if (file.bad() || file.fail())
		{
			GLHELPER_LOG_ERROR("Unable to open shader file " + shaderFilename);
			return "";
		}

		_allReadFiles.insert(shaderFilename);

		// Reserve
		std::string sourceCode;
		file.seekg(0, std::ios::end);
		sourceCode.reserve(static_cast<size_t>(file.tellg()));
		file.seekg(0, std::ios::beg);

		// Read
		sourceCode.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();

		std::string insertionBuffer;
		size_t parseCursorPos = 0;
		size_t parseCursorOriginalFileNumber = 1;
		size_t versionPos = sourceCode.find("#version");

		// Add #line macro for proper error output (see http://stackoverflow.com/questions/18176321/is-line-0-valid-in-glsl)
		// The big problem: Officially you can only give a number as second argument, no shader filename 
		// Don't insert one if this is the main file, recognizable by a #version tag!
		if (versionPos == std::string::npos)
		{
			insertionBuffer = "#line 1 " + std::to_string(fileIndex) + "\n";
			sourceCode.insert(0, insertionBuffer);
			parseCursorPos = insertionBuffer.size();
			parseCursorOriginalFileNumber = 1;
		}

		unsigned int lastFileIndex = fileIndex;

		// Prefix code (optional)
		if (!prefixCode.empty())
		{
			if (versionPos != std::string::npos)
			{
				// Insert after version and surround by #line macro for proper error output.
				size_t nextLineIdx = sourceCode.find_first_of("\n", versionPos);
				size_t numLinesBeforeVersion = std::count(sourceCode.begin(), sourceCode.begin() + versionPos, '\n');

				insertionBuffer = "\n#line 1 " + std::to_string(++lastFileIndex) + "\n";
				insertionBuffer += prefixCode;
				insertionBuffer += "\n#line " + std::to_string(numLinesBeforeVersion + 1) + " " + std::to_string(fileIndex) + "\n";

				sourceCode.insert(nextLineIdx, insertionBuffer);

				// parse cursor moves on
				parseCursorPos = nextLineIdx + insertionBuffer.size(); // This is the main reason why currently no #include files in prefix code is supported. Changing these has a lot of side effects in line numbering!
				parseCursorOriginalFileNumber = numLinesBeforeVersion + 1; // jumped over #version!
			}
		}

		// By adding this file to a NEW list of included files we allow multiple inclusion of the same file but disallow cycles.
		// Including the same file multiple times may be useful in some cases!
		auto includedFilesNew = _beforeIncludedFiles;
		includedFilesNew.emplace(shaderFilename);

		// parse all include tags
		size_t includePos = sourceCode.find("#include", parseCursorPos);
		std::string relativePath = PathUtils::GetDirectory(shaderFilename);
		while (includePos != std::string::npos)
		{
			parseCursorOriginalFileNumber += std::count(sourceCode.begin() + parseCursorPos, sourceCode.begin() + includePos, '\n');
			parseCursorPos = includePos;

			// parse filepath
			size_t quotMarksFirst = sourceCode.find("\"", includePos);
			if (quotMarksFirst == std::string::npos)
			{
				GLHELPER_LOG_ERROR("Invalid #include directive in shader file " + shaderFilename + ". Expected \"");
				break;
			}
			size_t quotMarksLast = sourceCode.find("\"", quotMarksFirst + 1);
			if (quotMarksFirst == std::string::npos)
			{
				GLHELPER_LOG_ERROR("Invalid #include directive in shader file " + shaderFilename + ". Expected \"");
				break;
			}

			size_t stringLength = quotMarksLast - quotMarksFirst - 1;
			if (stringLength == 0)
			{
				GLHELPER_LOG_ERROR("Invalid #include directive in shader file " + shaderFilename + ". Quotation marks empty!");
				break;
			}


			std::string includeCommand = sourceCode.substr(quotMarksFirst + 1, stringLength);
			std::string includeFile = PathUtils::AppendPath(relativePath, includeCommand);

			// Check if already included, to avoid cycles.
			if (_beforeIncludedFiles.find(includeFile) != _beforeIncludedFiles.end())
			{
				sourceCode.replace(includePos, includePos - quotMarksLast + 1, "\n");
				// just do nothing...
			}
			else
			{
				insertionBuffer = ReadShaderFromFile(includeFile, "", ++lastFileIndex, includedFilesNew, _allReadFiles);
				insertionBuffer += "\n#line " + std::to_string(parseCursorOriginalFileNumber + 1) + " " + std::to_string(fileIndex); // whitespace replaces #include!
				sourceCode.replace(includePos, quotMarksLast - includePos + 1, insertionBuffer);

				parseCursorPos += insertionBuffer.size();
			}

			// find next include
			includePos = sourceCode.find("#include", parseCursorPos);
		}

		return sourceCode;
	}

	Result ShaderObject::AddShaderFromSource(ShaderType type, const std::string& sourceCode, const std::string& sOriginName)
	{
		Shader& shader = m_shader[static_cast<std::uint32_t>(type)];

		// create shader
		GLuint shaderObjectTemp = 0;
		switch (type)
		{
		case ShaderObject::ShaderType::VERTEX:
			shaderObjectTemp = GL_RET_CALL(glCreateShader, GL_VERTEX_SHADER);
			break;
		case ShaderObject::ShaderType::FRAGMENT:
			shaderObjectTemp = GL_RET_CALL(glCreateShader, GL_FRAGMENT_SHADER);
			break;
		case ShaderObject::ShaderType::EVALUATION:
			shaderObjectTemp = GL_RET_CALL(glCreateShader, GL_TESS_EVALUATION_SHADER);
			break;
		case ShaderObject::ShaderType::CONTROL:
			shaderObjectTemp = GL_RET_CALL(glCreateShader, GL_TESS_CONTROL_SHADER);
			break;
		case ShaderObject::ShaderType::GEOMETRY:
			shaderObjectTemp = GL_RET_CALL(glCreateShader, GL_GEOMETRY_SHADER);
			break;
		case ShaderObject::ShaderType::COMPUTE:
			shaderObjectTemp = GL_RET_CALL(glCreateShader, GL_COMPUTE_SHADER);
			break;

		default:
			GLHELPER_ASSERT(false, "Unknown shader type");
			break;
		}

		// compile shader
		const char* sourceRaw = sourceCode.c_str();
		GL_CALL(glShaderSource, shaderObjectTemp, 1, &sourceRaw, nullptr);	// attach shader code

		Result result = gl::CheckGLError("glShaderSource");
		if (result == Result::SUCCEEDED)
		{
			glCompileShader(shaderObjectTemp);								    // compile

			result = gl::CheckGLError("glCompileShader");
		}

		// gl get error seems to be unreliable - another check!
		if (result == Result::SUCCEEDED)
		{
			GLint shaderCompiled;
			GL_CALL(glGetShaderiv, shaderObjectTemp, GL_COMPILE_STATUS, &shaderCompiled);

			if (shaderCompiled == GL_FALSE)
				result = Result::FAILURE;
		}

		// log output
		PrintShaderInfoLog(shaderObjectTemp, sOriginName);

		// check result
		if (result == Result::SUCCEEDED)
		{
			// destroy old shader
			if (shader.loaded)
			{
				glDeleteShader(shader.shaderObject);
				shader.sOrigin = "";
			}

			// memorize new data only if loading successful - this way a failed reload won't affect anything
			shader.shaderObject = shaderObjectTemp;
			shader.sOrigin = sOriginName;

			// remove old associated files
			for (auto it = m_filesPerShaderType.begin(); it != m_filesPerShaderType.end(); ++it)
			{
				if (it->second == type)
				{
					it = m_filesPerShaderType.erase(it);
					if (it == m_filesPerShaderType.end())
						break;
				}
			}

			shader.loaded = true;
		}
		else
			GL_CALL(glDeleteShader, shaderObjectTemp);

		return result;
	}


	Result ShaderObject::CreateProgram()
	{
		// Create shader program
		GLuint tempProgram = GL_RET_CALL(glCreateProgram);

		// attach programs
		int numAttachedShader = 0;
		for (Shader& shader : m_shader)
		{
			if (shader.loaded)
			{
				GL_CALL(glAttachShader, tempProgram, shader.shaderObject);
				++numAttachedShader;
			}
		}
		GLHELPER_ASSERT(numAttachedShader > 0, "Need at least one shader to link a gl program!");

		// Link program
		glLinkProgram(tempProgram);
		Result result = gl::CheckGLError("glLinkProgram");

		// gl get error seems to be unreliable - another check!
		if (result == Result::SUCCEEDED)
		{
			GLint programLinked;
			GL_CALL(glGetProgramiv, tempProgram, GL_LINK_STATUS, &programLinked);

			if (programLinked == GL_FALSE)
				result = Result::FAILURE;
		}

		// debug output
		PrintProgramInfoLog(tempProgram);

		// check
		if (result == Result::SUCCEEDED)
		{
			// already a program there? destroy old one!
			if (m_containsAssembledProgram)
			{
				GL_CALL(glDeleteProgram, m_program);

				// clear meta information
				m_totalProgramInputCount = 0;
				m_totalProgramOutputCount = 0;
				m_globalUniformInfo.clear();
				m_uniformBlockInfos.clear();
				m_shaderStorageInfos.clear();
			}

			// memorize new data only if loading successful - this way a failed reload won't affect anything
			m_program = tempProgram;
			m_containsAssembledProgram = true;

			// get informations about the program
			QueryProgramInformations();

			return Result::SUCCEEDED;
		}
		else
			glDeleteProgram(tempProgram);


		return result;
	}

	void ShaderObject::QueryProgramInformations()
	{
		// query basic uniform & shader storage block infos
		QueryBlockInformations(m_uniformBlockInfos, GL_UNIFORM_BLOCK);
		QueryBlockInformations(m_shaderStorageInfos, GL_SHADER_STORAGE_BLOCK);

		// informations about uniforms ...
		GLint iTotalNumUniforms = 0;
		GL_CALL(glGetProgramInterfaceiv, m_program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &iTotalNumUniforms);
		const GLuint iNumQueriedUniformProps = 10;
		const GLenum pQueriedUniformProps[] = { GL_NAME_LENGTH, GL_TYPE, GL_ARRAY_SIZE, GL_OFFSET, GL_BLOCK_INDEX, GL_ARRAY_STRIDE, GL_MATRIX_STRIDE, GL_IS_ROW_MAJOR, GL_ATOMIC_COUNTER_BUFFER_INDEX, GL_LOCATION };
		GLint pRawUniformBlockInfoData[iNumQueriedUniformProps] = {};
		for (int blockIndex = 0; blockIndex < iTotalNumUniforms; ++blockIndex)
		{
			// general data
			GLsizei length = 0;
			GL_CALL(glGetProgramResourceiv, m_program, GL_UNIFORM, blockIndex, iNumQueriedUniformProps, pQueriedUniformProps, iNumQueriedUniformProps, &length, pRawUniformBlockInfoData);
			UniformVariableInfo uniformInfo;
			uniformInfo.type = static_cast<gl::ShaderVariableType>(pRawUniformBlockInfoData[1]);
			uniformInfo.arrayElementCount = static_cast<std::int32_t>(pRawUniformBlockInfoData[2]);
			uniformInfo.blockOffset = static_cast<std::int32_t>(pRawUniformBlockInfoData[3]);
			uniformInfo.arrayStride = static_cast<std::int32_t>(pRawUniformBlockInfoData[5]) * 4;
			uniformInfo.matrixStride = static_cast<std::int32_t>(pRawUniformBlockInfoData[6]);
			uniformInfo.rowMajor = pRawUniformBlockInfoData[7] > 0;
			uniformInfo.atomicCounterbufferIndex = pRawUniformBlockInfoData[8];
			uniformInfo.location = pRawUniformBlockInfoData[9];

			// name
			GLint actualNameLength = 0;
			std::string name;
			name.resize(pRawUniformBlockInfoData[0] + 1);
			GL_CALL(glGetProgramResourceName, m_program, GL_UNIFORM, static_cast<GLuint>(blockIndex), static_cast<GLsizei>(name.size()), &actualNameLength, &name[0]);
			name.resize(actualNameLength);

			// where to store (to which ubo block does this variable belong)
			if (pRawUniformBlockInfoData[4] < 0)
				m_globalUniformInfo.emplace(name, uniformInfo);
			else
			{
				for (auto it = m_uniformBlockInfos.begin(); it != m_uniformBlockInfos.end(); ++it)
				{
					if (it->second.internalBufferIndex == pRawUniformBlockInfoData[4])
					{
						it->second.variables.emplace(name, uniformInfo);
						break;
					}
				}
			}
		}

		// informations about shader storage variables 
		GLint iTotalNumStorages = 0;
		GL_CALL(glGetProgramInterfaceiv, m_program, GL_BUFFER_VARIABLE, GL_ACTIVE_RESOURCES, &iTotalNumStorages);
		const GLuint numQueriedStorageProps = 10;
		const GLenum queriedStorageProps[] = { GL_NAME_LENGTH, GL_TYPE, GL_ARRAY_SIZE, GL_OFFSET, GL_BLOCK_INDEX, GL_ARRAY_STRIDE, GL_MATRIX_STRIDE, GL_IS_ROW_MAJOR, GL_TOP_LEVEL_ARRAY_SIZE, GL_TOP_LEVEL_ARRAY_STRIDE };
		GLint pRawStorageBlockInfoData[numQueriedStorageProps];
		for (GLint blockIndex = 0; blockIndex < iTotalNumStorages; ++blockIndex)
		{
			// general data
			GL_CALL(glGetProgramResourceiv, m_program, GL_BUFFER_VARIABLE, blockIndex, numQueriedStorageProps, queriedStorageProps, numQueriedStorageProps, nullptr, pRawStorageBlockInfoData);
			BufferVariableInfo storageInfo;
			storageInfo.type = static_cast<gl::ShaderVariableType>(pRawStorageBlockInfoData[1]);
			storageInfo.arrayElementCount = static_cast<std::int32_t>(pRawStorageBlockInfoData[2]);
			storageInfo.blockOffset = static_cast<std::int32_t>(pRawStorageBlockInfoData[3]);
			storageInfo.arrayStride = static_cast<std::int32_t>(pRawStorageBlockInfoData[5]);
			storageInfo.matrixStride = static_cast<std::int32_t>(pRawStorageBlockInfoData[6]);
			storageInfo.rowMajor = pRawStorageBlockInfoData[7] > 0;
			storageInfo.topLevelArraySize = pRawStorageBlockInfoData[8];
			storageInfo.topLevelArrayStride = pRawStorageBlockInfoData[9];

			// name
			GLint iActualNameLength = 0;
			std::string name;
			name.resize(pRawStorageBlockInfoData[0] + 1);
			GL_CALL(glGetProgramResourceName, m_program, GL_BUFFER_VARIABLE, blockIndex, static_cast<GLsizei>(name.size()), &iActualNameLength, &name[0]);
			name.resize(iActualNameLength);

			// where to store (to which shader storage block does this variable belong)
			for (auto it = m_shaderStorageInfos.begin(); it != m_shaderStorageInfos.end(); ++it)
			{
				if (it->second.internalBufferIndex == pRawStorageBlockInfoData[4])
				{
					it->second.variables.emplace(name, storageInfo);
					break;
				}
			}
		}

		// other informations
		GL_CALL(glGetProgramInterfaceiv, m_program, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &m_totalProgramInputCount);
		GL_CALL(glGetProgramInterfaceiv, m_program, GL_PROGRAM_OUTPUT, GL_ACTIVE_RESOURCES, &m_totalProgramOutputCount);
	}

	template<typename BufferVariableType>
	void ShaderObject::QueryBlockInformations(std::unordered_map<std::string, BufferInfo<BufferVariableType>>& BufferToFill, GLenum InterfaceName)
	{
		BufferToFill.clear();

		GLint iTotalNumBlocks = 0;
		GL_CALL(glGetProgramInterfaceiv, m_program, InterfaceName, GL_ACTIVE_RESOURCES, &iTotalNumBlocks);

		// gather infos about uniform blocks
		const GLuint iNumQueriedBlockProps = 4;
		const GLenum pQueriedBlockProps[] = { GL_NAME_LENGTH, GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE, GL_NUM_ACTIVE_VARIABLES };
		GLint pRawUniformBlockInfoData[iNumQueriedBlockProps];
		for (int blockIndex = 0; blockIndex < iTotalNumBlocks; ++blockIndex)
		{
			// general data
			GLsizei length = 0;
			GL_CALL(glGetProgramResourceiv, m_program, InterfaceName, blockIndex, iNumQueriedBlockProps, pQueriedBlockProps, iNumQueriedBlockProps, &length, pRawUniformBlockInfoData);
			BufferInfo<BufferVariableType> BlockInfo;
			BlockInfo.internalBufferIndex = blockIndex;
			BlockInfo.bufferBinding = pRawUniformBlockInfoData[1];
			BlockInfo.bufferDataSizeByte = pRawUniformBlockInfoData[2];// * sizeof(float);
			//BlockInfo.Variables.Reserve(pRawUniformBlockInfoData[3]);

			// name
			GLint actualNameLength = 0;
			std::string name;
			name.resize(pRawUniformBlockInfoData[0] + 1);
			GL_CALL(glGetProgramResourceName, m_program, InterfaceName, blockIndex, static_cast<GLsizei>(name.size()), &actualNameLength, &name[0]);
			name.resize(actualNameLength);

			BufferToFill.emplace(name, BlockInfo);
		}
	}

	GLuint ShaderObject::GetProgram() const
	{
		GLHELPER_ASSERT(m_containsAssembledProgram, "No shader program ready yet for ShaderObject \"" + m_name+ "\". Call CreateProgram first!");
		return m_program;
	}

	void ShaderObject::Activate() const
	{
		GLHELPER_ASSERT(m_containsAssembledProgram, "No shader program ready yet for ShaderObject \"" + m_name + "\". Call CreateProgram first!");
		GL_CALL(glUseProgram, m_program);
		s_currentlyActiveShaderObject = this;
	}

	Result ShaderObject::BindUBO(Buffer& _ubo, const std::string& _UBOName) const
	{
		auto it = m_uniformBlockInfos.find(_UBOName);
		if (it == m_uniformBlockInfos.end())
			return Result::FAILURE;

		_ubo.BindUniformBuffer(it->second.bufferBinding);

		return Result::SUCCEEDED;
	}

	Result ShaderObject::BindSSBO(gl::Buffer& _ssbo, const std::string& _SSBOName) const
	{
		auto storageBufferInfoIterator = GetShaderStorageBufferInfo().find(_SSBOName);
		if(storageBufferInfoIterator == GetShaderStorageBufferInfo().end())
		{
			GLHELPER_LOG_ERROR("Shader \"" + GetName() + "\" doesn't contain a storage buffer meta block info with the name \"" + _SSBOName + "\"!");
			return Result::FAILURE;
		}
		_ssbo.BindShaderStorageBuffer(storageBufferInfoIterator->second.bufferBinding);

		return Result::SUCCEEDED;
	}

	/*
	Result ShaderObject::BindImage(Texture& texture, Texture::ImageAccess accessMode, const std::string& sImageName)
	{
	auto it = m_GlobalUniformInfo.Find(sImageName);
	if(!it.IsValid())
	return Result::FAILURE;

	// optional typechecking
	switch(it.Value().Type)
	{
	case ShaderVariableType::UNSIGNED_INT_IMAGE_3D:
	case ShaderVariableType::INT_IMAGE_3D:
	case ShaderVariableType::IMAGE_3D:
	GLHELPER_ASSERT(dynamic_cast<Texture3D*>(&texture) != NULL, "3D Texture expected!");
	break;
	default:
	GLHELPER_ASSERT(false, "Handling for this type of uniform not implemented!");
	break;
	}

	GLHELPER_ASSERT(it.Value(). ??  >= 0, "Location of shader variable %s is invalid. You need to to specify the location with the layout qualifier!", it.Key());

	texture.BindImage(it.Value(). ??, accessMode);

	return EZ_SUCCESS;
	}

	Result ShaderObject::BindTexture(Texture& texture, const std::string& sTextureName)
	{
	auto it = m_GlobalUniformInfo.Find(sTextureName);
	if(!it.IsValid())
	return Result::FAILURE;

	// optional typechecking
	switch(it.Value().Type)
	{
	case ShaderVariableType::UNSIGNED_INT_SAMPLER_3D:
	case ShaderVariableType::INT_SAMPLER_3D:
	case ShaderVariableType::SAMPLER_3D:
	GLHELPER_ASSERT(dynamic_cast<Texture3D*>(&texture) != NULL, "3D Texture expected!");
	break;
	default:
	GLHELPER_ASSERT(false, "Handling for this type of uniform not implemented!");
	break;
	}

	GLHELPER_ASSERT(it.Value(). ?? >= 0, "Location of shader variable %s is invalid. You need to to specify the location with the layout qualifier!", it.Key());

	texture.Bind(it.Value(). ??);

	return EZ_SUCCESS;
	}
	*/

	void ShaderObject::PrintShaderInfoLog(ShaderId Shader, const std::string& sShaderName)
	{
#ifdef SHADER_COMPILE_LOGS
		GLint infologLength = 0;
		GLsizei charsWritten = 0;

		GL_CALL(glGetShaderiv, Shader, GL_INFO_LOG_LENGTH, &infologLength);
		std::string infoLog;
		infoLog.resize(infologLength);
		GL_CALL(glGetShaderInfoLog, Shader, infologLength, &charsWritten, &infoLog[0]);
		infoLog.resize(charsWritten);

		if (infoLog.size() > 0)
		{
			GLHELPER_LOG_ERROR("ShaderObject \"" + m_name + "\": Shader " + sShaderName + " compiled.Output:"); // Not necessarily an error - depends on driver.
			GLHELPER_LOG_ERROR(infoLog);
		}
		else
			GLHELPER_LOG_INFO("ShaderObject \"" + m_name + "\": Shader " + sShaderName + " compiled successfully");
#endif
	}

	// Print information about the linking step
	void ShaderObject::PrintProgramInfoLog(ProgramId Program)
	{
#ifdef SHADER_COMPILE_LOGS
		GLint infologLength = 0;
		GLsizei charsWritten = 0;

		GL_CALL(glGetProgramiv, Program, GL_INFO_LOG_LENGTH, &infologLength);
		std::string infoLog;
		infoLog.resize(infologLength);
		GL_CALL(glGetProgramInfoLog, Program, infologLength, &charsWritten, &infoLog[0]);
		infoLog.resize(charsWritten);

		if (infoLog.size() > 0)
		{
			GLHELPER_LOG_ERROR("Program \"" + m_name + "\" linked. Output:"); // Not necessarily an error - depends on driver.
			GLHELPER_LOG_ERROR(infoLog);
		}
		else
			GLHELPER_LOG_INFO("Program \"" + m_name + "\" linked successfully");
#endif
	}

	void ShaderObject::ShaderFileChangeHandler(const std::string& changedShaderFile)
	{
		auto it = m_filesPerShaderType.find(changedShaderFile);
		if (it != m_filesPerShaderType.end())
		{
			if (m_shader[static_cast<std::uint32_t>(it->second)].loaded)
			{
				std::string origin(m_shader[static_cast<std::uint32_t>(it->second)].sOrigin);  // need to copy the string, since it could be deleted in the course of reloading..
				if (AddShaderFromFile(it->second, origin) != Result::FAILURE && m_containsAssembledProgram)
					CreateProgram();
			}
		}
	}

	std::vector<char> ShaderObject::GetProgramBinary(GLenum& _binaryFormat)
	{
		GLHELPER_ASSERT(m_program != 0, "Program not yet compiled.");
		GLint binarySize = 0;
		GL_CALL(glGetProgramiv, m_program, GL_PROGRAM_BINARY_LENGTH, &binarySize);
		std::vector<char> data;
		data.resize(binarySize);

		GL_CALL(glGetProgramBinary, m_program, static_cast<GLsizei>(data.size()), nullptr, &_binaryFormat, data.data());
		
		return data;
	}

	/* void ShaderObject::ResetShaderBinding()
	 {
	 glUseProgram(0);
	 g_pCurrentlyActiveShaderObject == NULL;
	 } */
}