#pragma once

// ═══════════════════════════════════════════════════════════════════
//  Texture.h — GPU 텍스처 리소스
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"

class Texture
{
public:
    void LoadFromFile( ID3D12Device* device
                     , ID3D12GraphicsCommandList* cmdList
                     , const std::wstring& filePath
                     , D3D12_CPU_DESCRIPTOR_HANDLE srvHandle );

    void CreateWhite( ID3D12Device* device
                    , ID3D12GraphicsCommandList* cmdList
                    , D3D12_CPU_DESCRIPTOR_HANDLE srvHandle );

    void ReleaseUploadBuffer();

    bool IsValid() const;

    UINT SRVIndex = UINT_MAX;

private:
    void CreateSRV( ID3D12Device* device
                  , D3D12_CPU_DESCRIPTOR_HANDLE srvHandle );

    ComPtr<ID3D12Resource> m_textureGPU;
    ComPtr<ID3D12Resource> m_uploadBuffer;
};
