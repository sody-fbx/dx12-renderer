#pragma once

// ═══════════════════════════════════════════════════════════════════
//  ShaderUtil.h
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"

// X(ShaderName, FilePath)
#define SHADER_LIST \
    X(DEFAULT,    L"src/Shaders/Default.hlsl")           \
    X(TRIANGLE,   L"src/Shaders/Triangle.hlsl")          \
    X(LIGHT_BLPH, L"src/Shaders/Light_BlinnPhong.hlsl")  \
    X(SHADOW,     L"src/Shaders/Shadow.hlsl")            \
    X(PBR_TEX,    L"src/Shaders/PBR_Tex.hlsl")          \
    X(GBUFFER,    L"src/Shaders/GBuffer.hlsl")           \
    X(LIGHTING,   L"src/Shaders/Lighting.hlsl")          \

enum class SHADERTYPE : int
{
#define X(name, path) name,
    SHADER_LIST
#undef X
    COUNT
};

constexpr const wchar_t* ShaderPaths[] =
{
#define X(name, path) path,
    SHADER_LIST
#undef X
};

constexpr const char* ShaderNames[] =
{
#define X(name, path) #name,
    SHADER_LIST
#undef X
};

inline constexpr const wchar_t* GetShaderPath(SHADERTYPE type)
{
    return ShaderPaths[static_cast<int>(type)];
}

inline constexpr const char* GetShaderName(SHADERTYPE type)
{
    return ShaderNames[static_cast<int>(type)];
}

namespace Shader
{
    inline ComPtr<ID3DBlob> Compile ( const std::wstring& path
                                    , const std::string& entryPoint
                                    , const std::string& target )
    {
        UINT flags = 0;
#if defined(_DEBUG)
        flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        ComPtr<ID3DBlob> bytecode;
        ComPtr<ID3DBlob> errors;

    HRESULT hr = D3DCompileFromFile ( path.c_str()
                                    , nullptr       // Defines
                                    , D3D_COMPILE_STANDARD_FILE_INCLUDE
                                    , entryPoint.c_str()
                                    , target.c_str()
                                    , flags
                                    , 0
                                    , &bytecode
                                    , &errors );

        if (errors)
        {
            OutputDebugStringA(static_cast<const char*>(errors->GetBufferPointer()));
        }
        ThrowIfFailed(hr);

        return bytecode;
    }
}
