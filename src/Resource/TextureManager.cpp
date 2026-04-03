// ═══════════════════════════════════════════════════════════════════
//  TextureManager.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Resource/TextureManager.h"

void TextureManager::Request(const std::string& name, const std::wstring& path)
{
    // 이미 요청된 이름이면 무시
    for (auto& [n, p] : m_requests)
    {
        if (n == name) return;
    }
    m_requests.push_back({ name, path });
}

void TextureManager::BuildAll( ID3D12Device* device
                             , ID3D12GraphicsCommandList* cmdList
                             , DescriptorHeap& srvHeap )
{
    // [0] Fallback: 흰색 1×1 텍스처
    m_fallback = std::make_unique<Texture>();
    m_fallback->SRVIndex = srvHeap.AllocateIndex();
    m_fallback->CreateWhite(device, cmdList, srvHeap.GetCPUHandle(m_fallback->SRVIndex));

    // [1] Flat Normal: (128, 128, 255, 255) 1×1 텍스처
    m_flatNormal = std::make_unique<Texture>();
    m_flatNormal->SRVIndex = srvHeap.AllocateIndex();
    m_flatNormal->CreateFlatNormal(device, cmdList, srvHeap.GetCPUHandle(m_flatNormal->SRVIndex));

    // [1..N] 요청된 텍스처
    for (auto& [name, path] : m_requests)
    {
        auto tex = std::make_unique<Texture>();

        // Heap에서 다음 슬롯 할당
        tex->SRVIndex = srvHeap.AllocateIndex();

        tex->LoadFromFile(device, cmdList, path, srvHeap.GetCPUHandle(tex->SRVIndex));

        if (tex->IsValid())
        {
            m_textures[name] = std::move(tex);
        }
        else
        {
            // 로드 실패 시 슬롯은 비어있음 — Get()에서 fallback 반환
            OutputDebugStringA("[TextureManager] Load failed, will use fallback: ");
            OutputDebugStringA(name.c_str());
            OutputDebugStringA("\n");
        }
    }

    m_requests.clear();
    m_built = true;
}

void TextureManager::ReleaseUploadBuffers()
{
    if (m_fallback)
        m_fallback->ReleaseUploadBuffer();

    if (m_flatNormal)
        m_flatNormal->ReleaseUploadBuffer();

    for (auto& [name, tex] : m_textures)
        tex->ReleaseUploadBuffer();
}

Texture* TextureManager::Get(const std::string& name) const
{
    auto it = m_textures.find(name);
    if (it != m_textures.end() && it->second->IsValid())
        return it->second.get();

    // 없거나 로드 실패 -> fallback
    return m_fallback.get();
}

bool TextureManager::IsReady() const
{ 
    return m_built; 
}
