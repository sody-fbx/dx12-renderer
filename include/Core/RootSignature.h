#pragma once

// ═══════════════════════════════════════════════════════════════════
//  RootSignature.h — 셰이더-리소스 바인딩 레이아웃
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"

class RootSignature
{
public:
    // Resource X
    void CreateEmpty(ID3D12Device* device);

    // Resource = CBV
    void CreateWithCBV(ID3D12Device* device);

public: // Getter
    ID3D12RootSignature* Get() const { return m_rootSignature.Get(); }

private:
    ComPtr<ID3D12RootSignature> m_rootSignature;
};
