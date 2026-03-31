#pragma once

// ═══════════════════════════════════════════════════════════════════
//  PipelineState.h — Pipeline State Object (PSO) 관리
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"
#include "Shaders/ShaderUtil.h"

class PipelineState
{
public:
    void CreateTriangle ( ID3D12Device* device
                        , ID3D12RootSignature* rootSignature
                        , const std::wstring& vsPath
                        , const std::wstring& psPath
                        , DXGI_FORMAT backBufferFormat
                        , DXGI_FORMAT depthFormat );

    void CreateDefault (ID3D12Device* device
                       , ID3D12RootSignature* rootSignature
                       , const std::wstring& vsPath
                       , const std::wstring& psPath
                       , DXGI_FORMAT backBufferFormat
                       , DXGI_FORMAT depthFormat );

    void CreateShadow ( ID3D12Device* device
                      , ID3D12RootSignature* rootSignature
                      , const std::wstring& vsPath
                      , DXGI_FORMAT depthFormat );

    void CreateGBuffer( ID3D12Device* device
                      , ID3D12RootSignature* rootSignature
                      , const std::wstring& vsPath
                      , const std::wstring& psPath
                      , const DXGI_FORMAT rtFormats[3]
                      , DXGI_FORMAT depthFormat );

    void CreateLighting( ID3D12Device* device
                       , ID3D12RootSignature* rootSignature
                       , const std::wstring& vsPath
                       , const std::wstring& psPath
                       , DXGI_FORMAT backBufferFormat );

    ID3D12PipelineState* Get() const;

private:
    ComPtr<ID3D12PipelineState> m_pso;
};
