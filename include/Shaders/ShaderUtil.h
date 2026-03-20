#pragma once

// ═══════════════════════════════════════════════════════════════════
//  ShaderUtil.h
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"
#include "Shaders/ShaderList.h"

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
