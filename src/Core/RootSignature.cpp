// ═══════════════════════════════════════════════════════════════════
//  RootSignature.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Core/RootSignature.h"

void RootSignature::CreateEmpty(ID3D12Device* device)
{
    D3D12_ROOT_SIGNATURE_DESC desc  = {};
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
    rootParams[0].ParameterType                 = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[0].Descriptor.ShaderRegister     = 0;   // b0
    rootParams[0].Descriptor.RegisterSpace      = 0;
    rootParams[0].ShaderVisibility              = D3D12_SHADER_VISIBILITY_VERTEX;

    // [1] PassCB — register(b1)
    rootParams[1].ParameterType                 = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[1].Descriptor.ShaderRegister     = 1;   // b1
    rootParams[1].Descriptor.RegisterSpace      = 0;
    rootParams[1].ShaderVisibility              = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC desc = {};
    desc.NumParameters      = 2;
    desc.pParameters        = rootParams;
    desc.NumStaticSamplers  = 0;
    desc.pStaticSamplers    = nullptr;
    desc.Flags              = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

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
    srvRange.RangeType              = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srvRange.NumDescriptors         = 1;
    srvRange.BaseShaderRegister     = 0;
    srvRange.RegisterSpace          = 0;
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

void RootSignature::CreateWithTexture(ID3D12Device* device)
{
    D3D12_ROOT_PARAMETER rootParams[4] = {};

    // [0] ObjectCB — b0 (VS)
    rootParams[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[0].Descriptor.ShaderRegister = 0;
    rootParams[0].Descriptor.RegisterSpace  = 0;
    rootParams[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX;

    // [1] PassCB — b1 (ALL)
    rootParams[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[1].Descriptor.ShaderRegister = 1;
    rootParams[1].Descriptor.RegisterSpace  = 0;
    rootParams[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;

    // [2] Shadow Map SRV — t0 (PS)
    D3D12_DESCRIPTOR_RANGE shadowRange = {};
    shadowRange.RangeType                               = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    shadowRange.NumDescriptors                          = 1;
    shadowRange.BaseShaderRegister                      = 0;    // t0
    shadowRange.RegisterSpace                           = 0;
    shadowRange.OffsetInDescriptorsFromTableStart       = 0;

    rootParams[2].ParameterType                         = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[2].DescriptorTable.NumDescriptorRanges   = 1;
    rootParams[2].DescriptorTable.pDescriptorRanges     = &shadowRange;
    rootParams[2].ShaderVisibility                      = D3D12_SHADER_VISIBILITY_PIXEL;

    // [3] Albedo Texture SRV — t1 (PS)
    D3D12_DESCRIPTOR_RANGE texRange = {};
    texRange.RangeType                                  = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    texRange.NumDescriptors                             = 1;
    texRange.BaseShaderRegister                         = 1;    // t1
    texRange.RegisterSpace                              = 0;
    texRange.OffsetInDescriptorsFromTableStart          = 0;

    rootParams[3].ParameterType                         = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[3].DescriptorTable.NumDescriptorRanges   = 1;
    rootParams[3].DescriptorTable.pDescriptorRanges     = &texRange;
    rootParams[3].ShaderVisibility                      = D3D12_SHADER_VISIBILITY_PIXEL;

    // Static Samplers
    D3D12_STATIC_SAMPLER_DESC samplers[2] = {};

    // s0 — Shadow Map Comparison Sampler (PCF)
    samplers[0].Filter              = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    samplers[0].AddressU            = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    samplers[0].AddressV            = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    samplers[0].AddressW            = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    samplers[0].ComparisonFunc      = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    samplers[0].BorderColor         = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    samplers[0].ShaderRegister      = 0;    // s0
    samplers[0].RegisterSpace       = 0;
    samplers[0].ShaderVisibility    = D3D12_SHADER_VISIBILITY_PIXEL;

    // s1 — Albedo Texture Wrap Sampler (Anisotropic)
    samplers[1].Filter              = D3D12_FILTER_ANISOTROPIC;
    samplers[1].AddressU            = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplers[1].AddressV            = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplers[1].AddressW            = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplers[1].MaxAnisotropy       = 8;
    samplers[1].ComparisonFunc      = D3D12_COMPARISON_FUNC_NEVER;
    samplers[1].MinLOD              = 0.0f;
    samplers[1].MaxLOD              = D3D12_FLOAT32_MAX;
    samplers[1].ShaderRegister      = 1;    // s1
    samplers[1].RegisterSpace       = 0;
    samplers[1].ShaderVisibility    = D3D12_SHADER_VISIBILITY_PIXEL;

    // Root Signature 생성
    D3D12_ROOT_SIGNATURE_DESC desc = {};
    desc.NumParameters      = 4;
    desc.pParameters        = rootParams;
    desc.NumStaticSamplers  = 2;
    desc.pStaticSamplers    = samplers;
    desc.Flags              = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> serialized, error;
    HRESULT hr = D3D12SerializeRootSignature( &desc
                                            , D3D_ROOT_SIGNATURE_VERSION_1
                                            , &serialized
                                            , &error);
    if (error) OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
    ThrowIfFailed(hr);

    ThrowIfFailed(device->CreateRootSignature( 0
                                             , serialized->GetBufferPointer()
                                             , serialized->GetBufferSize()
                                             , IID_PPV_ARGS(&m_rootSignature)) );
}

void RootSignature::CreateWithGBuffer(ID3D12Device* device)
{
    D3D12_ROOT_PARAMETER rootParams[3] = {};

    // [0] ObjectCB — b0 (VS)
    rootParams[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[0].Descriptor.ShaderRegister = 0;
    rootParams[0].Descriptor.RegisterSpace  = 0;
    rootParams[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX;

    // [1] PassCB — b1 (ALL)
    rootParams[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[1].Descriptor.ShaderRegister = 1;
    rootParams[1].Descriptor.RegisterSpace  = 0;
    rootParams[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;

    // [2] Albedo Texture SRV — t0 (PS)
    D3D12_DESCRIPTOR_RANGE texRange = {};
    texRange.RangeType                              = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    texRange.NumDescriptors                         = 1;
    texRange.BaseShaderRegister                     = 0;    // t0
    texRange.RegisterSpace                          = 0;
    texRange.OffsetInDescriptorsFromTableStart       = 0;

    rootParams[2].ParameterType                     = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[2].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[2].DescriptorTable.pDescriptorRanges = &texRange;
    rootParams[2].ShaderVisibility                  = D3D12_SHADER_VISIBILITY_PIXEL;

    // s0 — Anisotropic Wrap (Albedo Texture)
    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter           = D3D12_FILTER_ANISOTROPIC;
    sampler.AddressU         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressV         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressW         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.MaxAnisotropy    = 8;
    sampler.ComparisonFunc   = D3D12_COMPARISON_FUNC_NEVER;
    sampler.MinLOD           = 0.0f;
    sampler.MaxLOD           = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister   = 0;    // s0
    sampler.RegisterSpace    = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_ROOT_SIGNATURE_DESC desc = {};
    desc.NumParameters      = 3;
    desc.pParameters        = rootParams;
    desc.NumStaticSamplers  = 1;
    desc.pStaticSamplers    = &sampler;
    desc.Flags              = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> serialized, error;
    HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &error);
    if (error) OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
    ThrowIfFailed(hr);

    ThrowIfFailed(device->CreateRootSignature(0,
        serialized->GetBufferPointer(), serialized->GetBufferSize(),
        IID_PPV_ARGS(&m_rootSignature)));
}

void RootSignature::CreateWithLighting(ID3D12Device* device)
{
    D3D12_ROOT_PARAMETER rootParams[3] = {};

    // [0] PassCB — b0 (ALL)
    rootParams[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[0].Descriptor.ShaderRegister = 0;
    rootParams[0].Descriptor.RegisterSpace  = 0;
    rootParams[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;

    // [1] Shadow Map SRV — t0 (PS), 1 descriptor
    D3D12_DESCRIPTOR_RANGE shadowRange = {};
    shadowRange.RangeType                           = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    shadowRange.NumDescriptors                      = 1;
    shadowRange.BaseShaderRegister                  = 0;    // t0
    shadowRange.RegisterSpace                       = 0;
    shadowRange.OffsetInDescriptorsFromTableStart   = 0;

    rootParams[1].ParameterType                     = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[1].DescriptorTable.pDescriptorRanges = &shadowRange;
    rootParams[1].ShaderVisibility                  = D3D12_SHADER_VISIBILITY_PIXEL;

    // [2] G-Buffer SRVs — t1, t2, t3 (PS), 3 consecutive descriptors
    D3D12_DESCRIPTOR_RANGE gbufRange = {};
    gbufRange.RangeType                             = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    gbufRange.NumDescriptors                        = 3;
    gbufRange.BaseShaderRegister                    = 1;    // t1
    gbufRange.RegisterSpace                         = 0;
    gbufRange.OffsetInDescriptorsFromTableStart     = 0;

    rootParams[2].ParameterType                     = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[2].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[2].DescriptorTable.pDescriptorRanges = &gbufRange;
    rootParams[2].ShaderVisibility                  = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_STATIC_SAMPLER_DESC samplers[2] = {};

    // s0 — Shadow Map Comparison Sampler (PCF)
    samplers[0].Filter           = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    samplers[0].AddressU         = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    samplers[0].AddressV         = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    samplers[0].AddressW         = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    samplers[0].ComparisonFunc   = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    samplers[0].BorderColor      = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    samplers[0].ShaderRegister   = 0;    // s0
    samplers[0].RegisterSpace    = 0;
    samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    // s1 — G-Buffer Point Clamp Sampler
    samplers[1].Filter           = D3D12_FILTER_MIN_MAG_MIP_POINT;
    samplers[1].AddressU         = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[1].AddressV         = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[1].AddressW         = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[1].ComparisonFunc   = D3D12_COMPARISON_FUNC_NEVER;
    samplers[1].MinLOD           = 0.0f;
    samplers[1].MaxLOD           = D3D12_FLOAT32_MAX;
    samplers[1].ShaderRegister   = 1;    // s1
    samplers[1].RegisterSpace    = 0;
    samplers[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_ROOT_SIGNATURE_DESC desc = {};
    desc.NumParameters      = 3;
    desc.pParameters        = rootParams;
    desc.NumStaticSamplers  = 2;
    desc.pStaticSamplers    = samplers;
    desc.Flags              = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> serialized, error;
    HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &error);
    if (error) OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
    ThrowIfFailed(hr);

    ThrowIfFailed(device->CreateRootSignature(0,
        serialized->GetBufferPointer(), serialized->GetBufferSize(),
        IID_PPV_ARGS(&m_rootSignature)));
}

ID3D12RootSignature* RootSignature::Get() const
{
    return m_rootSignature.Get();
}
