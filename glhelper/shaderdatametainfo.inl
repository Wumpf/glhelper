template<typename VariableType>
inline void BufferInfoView<VariableType>::SetableVariable::Set(float f)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::FLOAT, "Variable type does not match!");
	Set(&f, sizeof(f));
}
template<typename VariableType>
inline void BufferInfoView<VariableType>::SetableVariable::Set(const gl::Vec2& v)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::FLOAT_VEC2, "Variable type does not match!");
	Set(&v, sizeof(v));
}
template<typename VariableType>
inline void BufferInfoView<VariableType>::SetableVariable::Set(const gl::Vec3& v)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::FLOAT_VEC3, "Variable type does not match!");
	Set(&v, sizeof(v));
}
template<typename VariableType>
inline void BufferInfoView<VariableType>::SetableVariable::Set(const gl::Vec4& v)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::FLOAT_VEC4, "Variable type does not match!");
	Set(&v, sizeof(v));
}
template<typename VariableType>
inline void BufferInfoView<VariableType>::SetableVariable::Set(const gl::Mat3& m)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::FLOAT_MAT3, "Variable type does not match!");
	Set(&m, sizeof(m));
}
template<typename VariableType>
inline void BufferInfoView<VariableType>::SetableVariable::Set(const gl::Mat4& m)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::FLOAT_MAT4, "Variable type does not match!");
	Set(&m, sizeof(m));
}
template<typename VariableType>
inline void BufferInfoView<VariableType>::SetableVariable::Set(double f)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::DOUBLE, "Variable type does not match!");
	Set(&f, sizeof(f));
}
template<typename VariableType>
inline void BufferInfoView<VariableType>::SetableVariable::Set(std::uint32_t ui)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::UNSIGNED_INT, "Variable type does not match!");
	Set(&ui, sizeof(ui));
}
template<typename VariableType>
inline void BufferInfoView<VariableType>::SetableVariable::Set(const gl::UVec2& v)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::UNSIGNED_INT_VEC2, "Variable type does not match!");
	Set(&v, sizeof(v));
}
template<typename VariableType>
inline void BufferInfoView<VariableType>::SetableVariable::Set(const gl::UVec3& v)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::UNSIGNED_INT_VEC3, "Variable type does not match!");
	Set(&v, sizeof(v));
}
template<typename VariableType>
inline void BufferInfoView<VariableType>::SetableVariable::Set(const gl::UVec4& v)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::UNSIGNED_INT_VEC4, "Variable type does not match!");
	Set(&v, sizeof(v));
}

template<typename VariableType>
inline void BufferInfoView<VariableType>::SetableVariable::Set(std::int32_t i)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::INT, "Variable type does not match!");
	Set(&i, sizeof(i));
}
template<typename VariableType>
inline void BufferInfoView<VariableType>::SetableVariable::Set(const gl::IVec2 &v)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::INT_VEC2, "Variable type does not match!");
	Set(&v, sizeof(v));
}
template<typename VariableType>
inline void BufferInfoView<VariableType>::SetableVariable::Set(const gl::IVec3& v)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::INT_VEC3, "Variable type does not match!");
	Set(&v, sizeof(v));
}
template<typename VariableType>
inline void BufferInfoView<VariableType>::SetableVariable::Set(const gl::IVec4& v)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::INT_VEC4, "Variable type does not match!");
	Set(&v, sizeof(v));
}

template<typename VariableType>
inline void BufferInfoView<VariableType>::SetableVariable::Set(const void* _data, GLsizei _sizeInBytes)
{
	GLHELPER_ASSERT(_sizeInBytes != 0, "Given size to set for buffer memory is 0.");
	GLHELPER_ASSERT(_data != NULL, "Data to set for buffer variable is nullptr.");
	GLHELPER_ASSERT(m_parentBuffer.m_bufferInfo.bufferDataSizeByte >= m_MetaInfo.blockOffset + _sizeInBytes, "Data to set for buffer variable is out of buffer's memory range");

	m_parentBuffer.m_setVariableFunction(_data, _sizeInBytes, m_MetaInfo.blockOffset);
}

template<typename VariableType>
inline MappedMemoryView<VariableType>::MappedMemoryView(const BufferInfo<VariableType>& _bufferInfo, void* _mappedMemory, GLsizei _mapOffset) :
	BufferInfoView<VariableType>(_bufferInfo, [_mappedMemory, _mapOffset](const void* _data, GLsizei _sizeInBytes, std::int32_t _offset) {
		GLHELPER_ASSERT(_offset >= _mapOffset, "Variable is outside of mapped memory area!");
		// TODO: Overflow check!
		memcpy(reinterpret_cast<char*>(_mappedMemory)+(_offset - _mapOffset), _data, _sizeInBytes);
	})
{}
