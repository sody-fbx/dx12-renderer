#pragma once

// =====================================================================
//  ShaderList.h
// =====================================================================

// X(ShaderName, FilePath)
#define SHADER_LIST \
    X(DEFAULT,     L"src/Shaders/Default.hlsl")         \
    X(TRIANGLE,    L"src/Shaders/Triangle.hlsl")        \


// ShaderType enum 생성
enum class SHADERTYPE : int
{
#define X(name, path) name,
    SHADER_LIST
#undef X
    COUNT
};

// 셰이더 경로 배열 생성
constexpr const wchar_t* ShaderPaths[] =
{
#define X(name, path) path,
    SHADER_LIST
#undef X
};

// 셰이더 이름 문자열 배열 생성
constexpr const char* ShaderNames[] =
{
#define X(name, path) #name,
    SHADER_LIST
#undef X
};

// enum -> 경로 변환 헬퍼 함수
inline constexpr const wchar_t* GetShaderPath(SHADERTYPE type)
{
    return ShaderPaths[static_cast<int>(type)];
}

// enum -> 이름 변환 헬퍼 함수
inline constexpr const char* GetShaderName(SHADERTYPE type)
{
    return ShaderNames[static_cast<int>(type)];
}
