// ═══════════════════════════════════════════════════════════════════
//  PipelineState.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Core/PipelineState.h"

ComPtr<ID3DBlob> PipelineState::CompileShader ( const std::wstring& path
                                              , const std::string& entryPoint
                                              , const std::string& target )
{
    // 셰이더 컴파일
    UINT compileFlags = 0;
#if defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> bytecode;
    ComPtr<ID3DBlob> errors;

    HRESULT hr = D3DCompileFromFile ( path.c_str()
                                    , nullptr       // Defines
                                    , D3D_COMPILE_STANDARD_FILE_INCLUDE
                                    , entryPoint.c_str()
                                    , target.c_str()
                                    , compileFlags
                                    , 0
                                    , &bytecode
                                    , &errors );

    if (errors)
    {
        OutputDebugStringA(static_cast<const char*>(errors->GetBufferPointer()));
    }
    ThrowIfFailed(hr);

    return bytecode;
}

void PipelineState::Create ( ID3D12Device* device
                           , ID3D12RootSignature* rootSignature
                           , const std::wstring& vsPath
                           , const std::wstring& psPath
                           , DXGI_FORMAT backBufferFormat
                           , DXGI_FORMAT depthFormat )
{
    // 셰이더 컴파일
    auto vsBytecode = CompileShader(vsPath, "VSMain", "vs_5_1");
    auto psBytecode = CompileShader(psPath, "PSMain", "ps_5_1");

    // Input Layout
    // Triangle.hlsl
    //D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    //{
    //    // SemanticName, Index, Format, Slot, Offset, Classification, InstanceStep
    //    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    //    { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    //};
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

    ThrowIfFailed(
        device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pso))
    );
}
