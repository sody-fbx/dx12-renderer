// ═══════════════════════════════════════════════════════════════════
//  RootSignature.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Core/RootSignature.h"

void RootSignature::CreateEmpty(ID3D12Device* device)
{
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
    HRESULT hr = D3D12SerializeRootSignature(&desc
                                            , D3D_ROOT_SIGNATURE_VERSION_1
                                            , &serializedRootSig
                                            , &errorBlob );

    if (errorBlob)
    {
        OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
    }
    ThrowIfFailed(hr);

    // Device에서 생성
    ThrowIfFailed(device->CreateRootSignature( 0
                                             , serializedRootSig->GetBufferPointer()
                                             , serializedRootSig->GetBufferSize()
                                             , IID_PPV_ARGS(&m_rootSignature)) );
}

void RootSignature::CreateWithCBV(ID3D12Device* device)
{
    D3D12_ROOT_PARAMETER rootParams[2] = {};

    // [0] ObjectCB — register(b0)
    rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[0].Descriptor.ShaderRegister = 0;   // b0
    rootParams[0].Descriptor.RegisterSpace = 0;
    rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    // [1] PassCB — register(b1)
    rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[1].Descriptor.ShaderRegister = 1;   // b1
    rootParams[1].Descriptor.RegisterSpace = 0;
    rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC desc = {};
    desc.NumParameters = 2;
    desc.pParameters = rootParams;
    desc.NumStaticSamplers = 0;
    desc.pStaticSamplers = nullptr;
    desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> serialized, error;
    HRESULT hr = D3D12SerializeRootSignature( &desc
                                            , D3D_ROOT_SIGNATURE_VERSION_1
                                            , &serialized
                                            , &error );
    if (error) OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
    ThrowIfFailed(hr);

    ThrowIfFailed(device->CreateRootSignature( 0
                                             , serialized->GetBufferPointer()
                                             , serialized->GetBufferSize()
                                             , IID_PPV_ARGS(&m_rootSignature)));
}

void RootSignature::CreateWithShadow(ID3D12Device* device)
{
    D3D12_ROOT_PARAMETER rootParams[3] = {};

    // [0] CBV = ObjectCB (b0)
    rootParams[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[0].Descriptor.ShaderRegister = 0;
    rootParams[0].Descriptor.RegisterSpace  = 0;
    rootParams[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX;

    // [1] CBV = PassCB   (b1)
    rootParams[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[1].Descriptor.ShaderRegister = 1;
    rootParams[1].Descriptor.RegisterSpace  = 0;
    rootParams[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_DESCRIPTOR_RANGE srvRange = {};
    srvRange.RangeType          = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srvRange.NumDescriptors     = 1;
    srvRange.BaseShaderRegister = 0;
    srvRange.RegisterSpace      = 0;
    srvRange.OffsetInDescriptorsFromTableStart = 0;

    // [2] Descriptor Table = Shadow Map SRV (t0)
    rootParams[2].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[2].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[2].DescriptorTable.pDescriptorRanges   = &srvRange;
    rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    // Comparison Sampler for Shadow Map PCF
    D3D12_STATIC_SAMPLER_DESC shadowSampler = {};
    shadowSampler.Filter           = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    shadowSampler.AddressU         = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    shadowSampler.AddressV         = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    shadowSampler.AddressW         = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    shadowSampler.ComparisonFunc   = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    shadowSampler.BorderColor      = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    shadowSampler.ShaderRegister   = 0;
    shadowSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_ROOT_SIGNATURE_DESC desc = {};
    desc.NumParameters     = 3;
    desc.pParameters       = rootParams;
    desc.NumStaticSamplers = 1;
    desc.pStaticSamplers   = &shadowSampler;
    desc.Flags             = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> serialized, error;
    HRESULT hr = D3D12SerializeRootSignature( &desc
                                            , D3D_ROOT_SIGNATURE_VERSION_1
                                            , &serialized
                                            , &error );
    if (error) OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
    ThrowIfFailed(hr);

    ThrowIfFailed(device->CreateRootSignature( 0
                                             , serialized->GetBufferPointer()
                                             , serialized->GetBufferSize()
                                             , IID_PPV_ARGS(&m_rootSignature)));
}
