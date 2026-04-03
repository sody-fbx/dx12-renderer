#pragma once

// ═══════════════════════════════════════════════════════════════════
//  TextureManager.h — 텍스처 리소스 관리자
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"
#include "Core/DescriptorHeap.h"

#include "Resource/Texture.h"

class TextureManager
{
public:
    void Request(const std::string& name, const std::wstring& path);

    void BuildAll( ID3D12Device* device
                 , ID3D12GraphicsCommandList* cmdList
                 , DescriptorHeap& srvHeap );

    void ReleaseUploadBuffers();

    bool IsReady() const;

public: // Getter
    Texture* Get(const std::string& name) const;
    UINT     GetFlatNormalSRVIndex() const { return m_flatNormal->SRVIndex; }

private:
    // 로드 요청 목록 (name, path)
    std::vector<std::pair<std::string, std::wstring>> m_requests;
    // 로드 완료된 텍스처 (name, Texture)
    std::unordered_map<std::string, std::unique_ptr<Texture>> m_textures;
    // 흰색 1×1 Fallback
    std::unique_ptr<Texture> m_fallback;
    // Flat Normal 1×1 Fallback
    std::unique_ptr<Texture> m_flatNormal;

    bool m_built = false;
};
