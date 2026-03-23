#pragma once

// ═══════════════════════════════════════════════════════════════════
//  RootSignature.h — 셰이더-리소스 바인딩 레이아웃
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"

enum ROOT_SIGNATURE_TYPE
{
    ROOT_SIGNATURE_TYPE_EMPTY       = 0,
    ROOT_SIGNATURE_TYPE_CBV         = 1,
    ROOT_SIGNATURE_TYPE_SHADOW      = 2,
};

class RootSignature
{
public:
    // Resource X
    void CreateEmpty(ID3D12Device* device);

    // Resource = CBV
    void CreateWithCBV(ID3D12Device* device);

    // Resource = CBV + Shadow Map SRV
    void CreateWithShadow(ID3D12Device* device);

public: // Getter
    ID3D12RootSignature* Get() const { return m_rootSignature.Get(); }

private:
    ComPtr<ID3D12RootSignature> m_rootSignature;
};
