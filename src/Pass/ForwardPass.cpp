// ═══════════════════════════════════════════════════════════════════
//  ForwardPass.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Pass/ForwardPass.h"

void ForwardPass::Setup(ID3D12Device* device, int width, int height)
{
    // PSO와 RootSig는 Renderer가 소유하고 Set 함수로 전달받음.
    // ForwardPass 자체적으로 생성할 리소스는 없음 (Step 1 시점).
    // Step 4에서 Shadow Map 관련 설정이 추가됨.
    AddShader(device, D3D12_ROOT_PARAMETER_TYPE_CBV, SHADERTYPE::DEFAULT);
    CreateDSV(device, width, height);
}

void ForwardPass::Execute(const FrameContext& ctx)
{
    auto* cmdList = ctx.CmdList;
    auto* curFrame = ctx.CurrentFrameResource;

    // Viewport & Scissor
    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width    = static_cast<float>(ctx.ScreenWidth);
    viewport.Height   = static_cast<float>(ctx.ScreenHeight);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    cmdList->RSSetViewports(1, &viewport);

    D3D12_RECT scissor = { 0, 0, ctx.ScreenWidth, ctx.ScreenHeight };
    cmdList->RSSetScissorRects(1, &scissor);

    // Clear RTV & DSV
    D3D12_CPU_DESCRIPTOR_HANDLE DSV = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
    const float clearColor[] = { 0.05f, 0.05f, 0.08f, 1.0f };
    cmdList->ClearRenderTargetView(ctx.RTV, clearColor, 0, nullptr);
    cmdList->ClearDepthStencilView(DSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // 렌더타겟 바인딩
    cmdList->OMSetRenderTargets(1, &ctx.RTV, FALSE, &DSV);

    // RootSignature & PSO
    cmdList->SetGraphicsRootSignature(m_shader.RootSig.Get());
    cmdList->SetPipelineState(m_shader.Pso.Get());

    // PassCB 바인딩
    cmdList->SetGraphicsRootConstantBufferView(1, curFrame->PassCB->GetElementGPUAddress(0));

    // ── Step 4: Shadow Map SRV 바인딩 ──
    //if (m_hasShadowMap && m_srvHeap)
    //{
    //    ID3D12DescriptorHeap* heaps[] = { m_srvHeap };
    //    cmdList->SetDescriptorHeaps(1, heaps);
    //    cmdList->SetGraphicsRootDescriptorTable(2, m_shadowMapSRV);
    //}

    // RenderItem(Mesh) Draw
    for (auto& item : *ctx.RenderItems)
    {
        // 이 오브젝트의 Mesh 바인딩
        auto vbv = item->MeshRef->VertexBufferView();
        auto ibv = item->MeshRef->IndexBufferView();
        cmdList->IASetVertexBuffers(0, 1, &vbv);
        cmdList->IASetIndexBuffer(&ibv);
        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // 이 오브젝트의 ObjectCB 바인딩
        cmdList->SetGraphicsRootConstantBufferView(0, curFrame->ObjectCB->GetElementGPUAddress(item->ObjCBIndex));

        cmdList->DrawIndexedInstanced(item->MeshRef->GetIndexCount(), 1, 0, 0, 0);
    }
}

void ForwardPass::OnResize(ID3D12Device* device, int width, int height)
{
    m_depthBuffer.Reset();
    // Depth Buffer 재생성
    CreateDSV(device, width, height);
}

void ForwardPass::AddShader( ID3D12Device* device
                           , D3D12_ROOT_PARAMETER_TYPE rootType
                           , SHADERTYPE shaderType)
{
    Shader shader;

    switch (rootType)
    {
    case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
        break;
    case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
        break;
    case D3D12_ROOT_PARAMETER_TYPE_CBV:
        shader.RootSig.CreateWithCBV(device);
        break;
    case D3D12_ROOT_PARAMETER_TYPE_SRV:
        break;
    case D3D12_ROOT_PARAMETER_TYPE_UAV:
        break;
    default:
        shader.RootSig.CreateEmpty(device);
        break;
    }
    
    std::wstring shaderPath;

    switch (shaderType)
    {
    case SHADERTYPE::DEFAULT:
        shaderPath = GetShaderPath(SHADERTYPE::DEFAULT);
        shader.Pso.CreateDefault( device
                                , shader.RootSig.Get()
                                , shaderPath, shaderPath
                                , BACK_BUFFER_FORMAT, DEPTH_FORMAT);
        break;
    case SHADERTYPE::TRIANGLE:
        shaderPath = GetShaderPath(SHADERTYPE::TRIANGLE);
        shader.Pso.CreateTriangle( device
                                 , shader.RootSig.Get()
                                 , shaderPath, shaderPath
                                 , BACK_BUFFER_FORMAT, DEPTH_FORMAT);
        break;
    default:
        break;
    }

    m_shader = shader;
}

void ForwardPass::CreateDSV(ID3D12Device* device, int width, int height)
{
    // Depth/Stencil Buffer
    D3D12_RESOURCE_DESC depthDesc = {};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format = DEPTH_FORMAT;
    depthDesc.SampleDesc = { 1, 0 };
    depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DEPTH_FORMAT;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;   // GPU 전용 메모리

    ThrowIfFailed(
        device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &depthDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,   // 초기 상태: Depth 쓰기
            &clearValue,
            IID_PPV_ARGS(&m_depthBuffer)
        )
    );

    // DSV (Depth Stencil View) Heap
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    ThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

    device->CreateDepthStencilView(
        m_depthBuffer.Get(), nullptr,
        m_dsvHeap->GetCPUDescriptorHandleForHeapStart()
    );
}