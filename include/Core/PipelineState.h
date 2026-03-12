#pragma once

// ═══════════════════════════════════════════════════════════════════
//  PipelineState.h — Pipeline State Object (PSO) 관리
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"

class PipelineState
{
public:
    // 기본 PSO 생성: 셰이더 파일 경로를 받아 컴파일 후 PSO 생성
    void Create( ID3D12Device* device
               , ID3D12RootSignature* rootSignature
               , const std::wstring& vsPath
               , const std::wstring& psPath
               , DXGI_FORMAT backBufferFormat
               , DXGI_FORMAT depthFormat );

    ID3D12PipelineState* Get() const { return m_pso.Get(); }

private:
    ComPtr<ID3DBlob> CompileShader( const std::wstring& path
                                  , const std::string& entryPoint
                                  , const std::string& target);

    ComPtr<ID3D12PipelineState> m_pso;
};
