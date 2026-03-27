// ═══════════════════════════════════════════════════════════════════
//  ForwardPass.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Pass/ForwardPass.h"

void ForwardPass::Setup(ID3D12Device* device, int width, int height,
                        const ShadowMap* shadowMap, const RenderContext* renderCtx)
{
    m_shadowMap  = shadowMap;
    m_renderCtx  = renderCtx;

    m_pipelineSet.Create(device, ROOT_SIGNATURE_TYPE_TEXTURE, SHADERTYPE::PBR_TEX);
    CreateDepthTarget(device, width, height);
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
    auto RTV = ctx.RTV;
    auto DSV = m_depth.DSV();
    const float clearColor[] = { 0.05f, 0.05f, 0.08f, 1.0f };
    cmdList->ClearRenderTargetView(RTV, clearColor, 0, nullptr);
    cmdList->ClearDepthStencilView(DSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // 렌더타겟 바인딩
    cmdList->OMSetRenderTargets(1, &RTV, FALSE, &DSV);

    // RootSignature & PSO
    cmdList->SetGraphicsRootSignature(m_pipelineSet.RootSignature.Get());
    cmdList->SetPipelineState(m_pipelineSet.Pso.Get());

    // [1] PassCB 바인딩
    cmdList->SetGraphicsRootConstantBufferView(1, curFrame->PassCB->GetElementGPUAddress(0));

    // SRV Heap 바인딩
    const DescriptorHeap* srvHeap = (m_renderCtx != nullptr) ? m_renderCtx->SrvHeap : nullptr;

    if (srvHeap != nullptr)
    {
        ID3D12DescriptorHeap* heaps[] = { srvHeap->Get() };
        cmdList->SetDescriptorHeaps(1, heaps);

        // [2] Shadow Map SRV 바인딩 (t0)
        if (m_shadowMap && m_shadowMap->IsValid())
        {
            cmdList->SetGraphicsRootDescriptorTable(
                2, srvHeap->GetGPUHandle(m_shadowMap->SRVIndex));
        }
    }

    const TextureManager* texMgr = (m_renderCtx != nullptr) ? m_renderCtx->Textures : nullptr;

    // RenderItem(Mesh) Draw
    for (auto& item : *ctx.RenderItems)
    {
        // [0] ObjectCB 바인딩
        cmdList->SetGraphicsRootConstantBufferView(0, curFrame->ObjectCB->GetElementGPUAddress(item->ObjCBIndex));

        // [3] Albedo Texture
        if (srvHeap != nullptr && texMgr != nullptr && item->TexRef != nullptr)
        {
            cmdList->SetGraphicsRootDescriptorTable(3, srvHeap->GetGPUHandle(item->TexRef->SRVIndex));
        }

        // Mesh 바인딩
        auto vertexView = item->MeshRef->VertexBufferView();
        auto indexView  = item->MeshRef->IndexBufferView();
        cmdList->IASetVertexBuffers(0, 1, &vertexView);
        cmdList->IASetIndexBuffer(&indexView);
        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        cmdList->DrawIndexedInstanced(item->MeshRef->GetIndexCount(), 1, 0, 0, 0);
    }
}

void ForwardPass::OnResize(ID3D12Device* device, int width, int height)
{
    // Depth 재생성
    m_depth.Buffer.Reset();
    CreateDepthTarget(device, width, height);
}

void ForwardPass::CreateDepthTarget(ID3D12Device* device, int width, int height)
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

    ThrowIfFailed(device->CreateCommittedResource( &heapProps, D3D12_HEAP_FLAG_NONE
                                                 , &depthDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE
                                                 , &clearValue, IID_PPV_ARGS(&m_depth.Buffer)));

    // DSV (Depth Stencil View) Heap
    m_depth.Heap.Initialize( device
                           , D3D12_DESCRIPTOR_HEAP_TYPE_DSV
                           , 1
                           , false );

    device->CreateDepthStencilView( m_depth.Buffer.Get()
                                  , nullptr
                                  , m_depth.Heap.AllocateHandle() );
}