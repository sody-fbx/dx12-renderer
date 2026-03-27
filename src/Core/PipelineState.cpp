// ═══════════════════════════════════════════════════════════════════
//  PipelineState.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Core/PipelineState.h"

void PipelineState::CreateTriangle( ID3D12Device* device
                                  , ID3D12RootSignature* rootSignature
                                  , const std::wstring& vsPath
                                  , const std::wstring& psPath
                                  , DXGI_FORMAT backBufferFormat
                                  , DXGI_FORMAT depthFormat )
{
    // 셰이더 컴파일
    auto vsBytecode = Shader::Compile(vsPath, "VSMain", "vs_5_1");
    auto psBytecode = Shader::Compile(psPath, "PSMain", "ps_5_1");

    // Input Layout
    // Triangle.hlsl
    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        // SemanticName, Index, Format, Slot, Offset, Classification, InstanceStep
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // PSO 생성
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

    // 셰이더
    psoDesc.pRootSignature = rootSignature;
    psoDesc.VS = { vsBytecode->GetBufferPointer(), vsBytecode->GetBufferSize() };
    psoDesc.PS = { psBytecode->GetBufferPointer(), psBytecode->GetBufferSize() };

    // Input Layout
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };

    // Rasterizer
    psoDesc.RasterizerState.FillMode              = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode              = D3D12_CULL_MODE_BACK;
    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    psoDesc.RasterizerState.DepthClipEnable       = TRUE;

    // Blend
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    // Depth/Stencil
    psoDesc.DepthStencilState.DepthEnable    = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc      = D3D12_COMPARISON_FUNC_LESS;

    // 샘플링
    psoDesc.SampleMask          = UINT_MAX;
    psoDesc.SampleDesc          = { 1, 0 };

    // 출력 포맷
    psoDesc.NumRenderTargets    = 1;
    psoDesc.RTVFormats[0]       = backBufferFormat;
    psoDesc.DSVFormat           = depthFormat;

    // 토폴로지
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    ThrowIfFailed(
        device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pso))
    );
}

void PipelineState::CreateDefault ( ID3D12Device* device
                                  , ID3D12RootSignature* rootSignature
                                  , const std::wstring& vsPath
                                  , const std::wstring& psPath
                                  , DXGI_FORMAT backBufferFormat
                                  , DXGI_FORMAT depthFormat )
{
    // 셰이더 컴파일
    auto vsBytecode = Shader::Compile(vsPath, "VSMain", "vs_5_1");
    auto psBytecode = Shader::Compile(psPath, "PSMain", "ps_5_1");

    // Input Layout
    // Default.hlsl
    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // PSO 생성
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

    // 셰이더
    psoDesc.pRootSignature = rootSignature;
    psoDesc.VS = { vsBytecode->GetBufferPointer(), vsBytecode->GetBufferSize() };
    psoDesc.PS = { psBytecode->GetBufferPointer(), psBytecode->GetBufferSize() };

    // Input Layout
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };

    // Rasterizer
    psoDesc.RasterizerState.FillMode              = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode              = D3D12_CULL_MODE_BACK;
    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    psoDesc.RasterizerState.DepthClipEnable       = TRUE;

    // Blend
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    // Depth/Stencil
    psoDesc.DepthStencilState.DepthEnable    = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc      = D3D12_COMPARISON_FUNC_LESS;

    // 샘플링
    psoDesc.SampleMask          = UINT_MAX;
    psoDesc.SampleDesc          = { 1, 0 };

    // 출력 포맷
    psoDesc.NumRenderTargets    = 1;
    psoDesc.RTVFormats[0]       = backBufferFormat;
    psoDesc.DSVFormat           = depthFormat;

    // 토폴로지
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pso)));
}

void PipelineState::CreateShadow ( ID3D12Device* device
                                 , ID3D12RootSignature* rootSignature
                                 , const std::wstring& vsPath
                                 , DXGI_FORMAT depthFormat )
{
    // 셰이더 컴파일
    // Depth-only 이기 때문에 VS만 컴파일
    auto vsBytecode = Shader::Compile(vsPath, "VSMain", "vs_5_1");

    // Input Layout
    // Shadow.hlsl
    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // PSO 생성
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

    // 셰이더
    psoDesc.pRootSignature = rootSignature;
    psoDesc.VS = { vsBytecode->GetBufferPointer(), vsBytecode->GetBufferSize() };
    // PS 없음

    // Input Layout
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };

    // Rasterizer
    psoDesc.RasterizerState.FillMode              = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode              = D3D12_CULL_MODE_BACK;
    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    psoDesc.RasterizerState.DepthBias             = 10000; // DepthBias로 Shadow Acne 방지
    psoDesc.RasterizerState.DepthBiasClamp        = 0.0f;
    psoDesc.RasterizerState.SlopeScaledDepthBias  = 1.0f;
    psoDesc.RasterizerState.DepthClipEnable       = TRUE;

    // Blend
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = 0; // 컬러 출력 없음!

    // Depth/Stencil
    psoDesc.DepthStencilState.DepthEnable    = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc      = D3D12_COMPARISON_FUNC_LESS;

    // 샘플링
    psoDesc.SampleMask             = UINT_MAX;
    psoDesc.SampleDesc             = { 1, 0 };

    // 출력 포맷
    psoDesc.NumRenderTargets       = 0;
    // RTV 없음
    psoDesc.DSVFormat              = depthFormat;

    // 토폴로지
    psoDesc.PrimitiveTopologyType  = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pso)));
}

ID3D12PipelineState* PipelineState::Get() const
{
    return m_pso.Get();
}
