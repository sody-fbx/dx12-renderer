#pragma once

// ═══════════════════════════════════════════════════════════════════
//  RootSignature.h — 셰이더-리소스 바인딩 레이아웃
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"

class RootSignature
{
public:
    // 빈 Root Signature (셰이더에 외부 데이터를 넘기지 않음)
    void CreateEmpty(ID3D12Device* device);

    // CBV 1개를 받는 Root Signature
    void CreateWithCBV(ID3D12Device* device);

    ID3D12RootSignature* Get() const { return m_rootSignature.Get(); }

private:
    ComPtr<ID3D12RootSignature> m_rootSignature;
};
