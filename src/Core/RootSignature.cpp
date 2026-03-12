// ═══════════════════════════════════════════════════════════════════
//  RootSignature.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Core/RootSignature.h"

void RootSignature::CreateEmpty(ID3D12Device* device)
{
    // TODO : MVP 행렬을 넘기려면 여기에 CBV Root Parameter를 추가.

    D3D12_ROOT_SIGNATURE_DESC desc = {};
    desc.NumParameters     = 0;
    desc.pParameters       = nullptr;
    desc.NumStaticSamplers = 0;
    desc.pStaticSamplers   = nullptr;
    desc.Flags             = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    // 정점 셰이더가 Input Layout을 사용함.

    // 직렬화
    ComPtr<ID3DBlob> serializedRootSig;
    ComPtr<ID3DBlob> errorBlob;

    // 바이트코드로 변환
    HRESULT hr = D3D12SerializeRootSignature(
        &desc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &serializedRootSig,
        &errorBlob
    );

    if (errorBlob)
    {
        OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
    }
    ThrowIfFailed(hr);

    // Device에서 생성
    ThrowIfFailed(
        device->CreateRootSignature(
            0,
            serializedRootSig->GetBufferPointer(),
            serializedRootSig->GetBufferSize(),
            IID_PPV_ARGS(&m_rootSignature)
        )
    );
}
