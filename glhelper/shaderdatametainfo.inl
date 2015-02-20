template<typename VariableType>
inline void ShaderVariable<VariableType>::Set(float f)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::FLOAT, "Variable type does not match!");
	Set(&f, sizeof(f));
}
template<typename VariableType>
inline void ShaderVariable<VariableType>::Set(const gl::Vec2& v)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::FLOAT_VEC2, "Variable type does not match!");
	Set(&v, sizeof(v));
}
template<typename VariableType>
inline void ShaderVariable<VariableType>::Set(const gl::Vec3& v)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::FLOAT_VEC3, "Variable type does not match!");
	Set(&v, sizeof(v));
}
template<typename VariableType>
inline void ShaderVariable<VariableType>::Set(const gl::Vec4& v)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::FLOAT_VEC4, "Variable type does not match!");
	Set(&v, sizeof(v));
}
template<typename VariableType>
inline void ShaderVariable<VariableType>::Set(const gl::Mat3& m)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::FLOAT_MAT3, "Variable type does not match!");
	Set(&m, sizeof(m));
}
template<typename VariableType>
inline void ShaderVariable<VariableType>::Set(const gl::Mat4& m)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::FLOAT_MAT4, "Variable type does not match!");
	Set(&m, sizeof(m));
}
template<typename VariableType>
inline void ShaderVariable<VariableType>::Set(double f)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::DOUBLE, "Variable type does not match!");
	Set(&f, sizeof(f));
}
template<typename VariableType>
inline void ShaderVariable<VariableType>::Set(std::uint32_t ui)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::UNSIGNED_INT, "Variable type does not match!");
	Set(&ui, sizeof(ui));
}
template<typename VariableType>
inline void ShaderVariable<VariableType>::Set(const gl::UVec2& v)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::UNSIGNED_INT_VEC2, "Variable type does not match!");
	Set(&v, sizeof(v));
}
template<typename VariableType>
inline void ShaderVariable<VariableType>::Set(const gl::UVec3& v)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::UNSIGNED_INT_VEC3, "Variable type does not match!");
	Set(&v, sizeof(v));
}
template<typename VariableType>
inline void ShaderVariable<VariableType>::Set(const gl::UVec4& v)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::UNSIGNED_INT_VEC4, "Variable type does not match!");
	Set(&v, sizeof(v));
}

template<typename VariableType>
inline void ShaderVariable<VariableType>::Set(std::int32_t i)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::INT, "Variable type does not match!");
	Set(&i, sizeof(i));
}
template<typename VariableType>
inline void ShaderVariable<VariableType>::Set(const gl::IVec2 &v)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::INT_VEC2, "Variable type does not match!");
	Set(&v, sizeof(v));
}
template<typename VariableType>
inline void ShaderVariable<VariableType>::Set(const gl::IVec3& v)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::INT_VEC3, "Variable type does not match!");
	Set(&v, sizeof(v));
}
template<typename VariableType>
inline void ShaderVariable<VariableType>::Set(const gl::IVec4& v)
{
	GLHELPER_ASSERT(m_MetaInfo.type == ShaderVariableType::INT_VEC4, "Variable type does not match!");
	Set(&v, sizeof(v));
}
