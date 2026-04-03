// ═══════════════════════════════════════════════════════════════════
//  GeometryPass.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Pass/GeometryPass.h"
#include "imgui.h"

void GeometryPass::Setup( ID3D12Device*        device
                        , int                  width
                        , int                  height
                        , DescriptorHeap&      srvHeap
                        , const RenderContext* renderCtx )
{
    m_srvHeap   = &srvHeap;
    m_renderCtx = renderCtx;

    // 공유 SRV Heap에서 G-Buffer용 슬롯 3개 연속 할당
    // LightingPass에서 단일 Descriptor Table(3개)로 바인딩하므로 연속이어야 함
    for (UINT i = 0; i < GBuffer::COUNT; i++)
        m_gBuffer.SRVIndices[i] = srvHeap.AllocateIndex();

    m_rootSig.CreateWithGBuffer(device);
    m_pso.CreateGBuffer( device
                       , m_rootSig.Get()
                       , GetShaderPath(SHADERTYPE::GBUFFER)
                       , GetShaderPath(SHADERTYPE::GBUFFER)
                       , GBuffer::RTFormats
                       , DEPTH_FORMAT );

    CreateGBufferTargets(device, width, height);
    CreateDepthTarget(device, width, height);
}

void GeometryPass::Execute(const FrameContext& ctx)
{
    auto* cmdList  = ctx.CmdList;
    auto* curFrame = ctx.CurrentFrameResource;

    // Barrier: PIXEL_SHADER_RESOURCE → RENDER_TARGET
    D3D12_RESOURCE_BARRIER barriers[GBuffer::COUNT] = {};
    for (UINT i = 0; i < GBuffer::COUNT; i++)
    {
        barriers[i].Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barriers[i].Transition.pResource   = m_gBuffer.Textures[i].Get();
        barriers[i].Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        barriers[i].Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barriers[i].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    }
    cmdList->ResourceBarrier(GBuffer::COUNT, barriers);

    // Viewport & Scissor
    D3D12_VIEWPORT vp = { 0, 0, (float)ctx.ScreenWidth, (float)ctx.ScreenHeight, 0, 1 };
    D3D12_RECT     sc = { 0, 0, ctx.ScreenWidth, ctx.ScreenHeight };
    cmdList->RSSetViewports(1, &vp);
    cmdList->RSSetScissorRects(1, &sc);

    // G-Buffer Clear
    const float clearBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    for (UINT i = 0; i < GBuffer::COUNT; i++)
        cmdList->ClearRenderTargetView(m_gBuffer.RTV(i), clearBlack, 0, nullptr);
    cmdList->ClearDepthStencilView(m_depth.DSV(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // MRT 바인딩
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[GBuffer::COUNT] = {
        m_gBuffer.RTV(0), m_gBuffer.RTV(1), m_gBuffer.RTV(2)
    };
    auto dsv = m_depth.DSV();
    cmdList->OMSetRenderTargets(GBuffer::COUNT, rtvHandles, FALSE, &dsv);

    // Root Signature & PSO
    cmdList->SetGraphicsRootSignature(m_rootSig.Get());
    cmdList->SetPipelineState(m_pso.Get());

    // [1] PassCB 바인딩
    cmdList->SetGraphicsRootConstantBufferView(1, curFrame->PassCB->GetElementGPUAddress(0));

    const DescriptorHeap* srvHeap = m_renderCtx ? m_renderCtx->SrvHeap : nullptr;
    const TextureManager* texMgr  = m_renderCtx ? m_renderCtx->Textures : nullptr;

    if (srvHeap)
    {
        ID3D12DescriptorHeap* heaps[] = { srvHeap->Get() };
        cmdList->SetDescriptorHeaps(1, heaps);
    }

    // RenderItem Draw
    for (auto& item : *ctx.RenderItems)
    {
        // [0] ObjectCB
        cmdList->SetGraphicsRootConstantBufferView(0, curFrame->ObjectCB->GetElementGPUAddress(item->ObjCBIndex));

        // [2] Albedo Texture
        if (srvHeap && texMgr && item->TexRef)
            cmdList->SetGraphicsRootDescriptorTable(2, srvHeap->GetGPUHandle(item->TexRef->SRVIndex));

        // [3] Normal Map
        if (srvHeap)
        {
            UINT normalSRV = (item->NormalMapRef != nullptr)
                           ? item->NormalMapRef->SRVIndex
                           : m_renderCtx->Textures->GetFlatNormalSRVIndex();
            cmdList->SetGraphicsRootDescriptorTable(3, srvHeap->GetGPUHandle(normalSRV));
        }

        auto vertexView = item->MeshRef->VertexBufferView();
        auto indexView  = item->MeshRef->IndexBufferView();
        cmdList->IASetVertexBuffers(0, 1, &vertexView);
        cmdList->IASetIndexBuffer(&indexView);
        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmdList->DrawIndexedInstanced(item->MeshRef->GetIndexCount(), 1, 0, 0, 0);
    }

    // Barrier: RENDER_TARGET → PIXEL_SHADER_RESOURCE
    for (UINT i = 0; i < GBuffer::COUNT; i++)
    {
        barriers[i].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barriers[i].Transition.StateAfter  = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    }
    cmdList->ResourceBarrier(GBuffer::COUNT, barriers);
}

void GeometryPass::OnResize(ID3D12Device* device, int width, int height)
{
    for (UINT i = 0; i < GBuffer::COUNT; i++)
        m_gBuffer.Textures[i].Reset();
    m_depth.Buffer.Reset();

    CreateGBufferTargets(device, width, height);
    CreateDepthTarget(device, width, height);
}

void GeometryPass::OnDrawDebugUI()
{
    if (ImGui::CollapsingHeader("Geometry Pass (Deferred)"))
    {
        ImGui::Text("G-Buffer Count : %u", GBuffer::COUNT);
        ImGui::Text("SRV Indices    : %u, %u, %u"
                   , m_gBuffer.SRVIndices[0]
                   , m_gBuffer.SRVIndices[1]
                   , m_gBuffer.SRVIndices[2]);
        ImGui::Text("[0] Albedo     : R8G8B8A8_UNORM");
        ImGui::Text("[1] Normal     : R16G16B16A16_FLOAT");
        ImGui::Text("[2] World Pos  : R16G16B16A16_FLOAT");
    }
}

void GeometryPass::CreateGBufferTargets(ID3D12Device* device, int width, int height)
{
    D3D12_HEAP_PROPERTIES heapProps = { D3D12_HEAP_TYPE_DEFAULT };

    // RTV Heap 재초기화
    m_gBuffer.RtvHeap.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, GBuffer::COUNT, false);

    for (UINT i = 0; i < GBuffer::COUNT; i++)
    {
        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension        = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width            = width;
        desc.Height           = height;
        desc.DepthOrArraySize = 1;
        desc.MipLevels        = 1;
        desc.Format           = GBuffer::RTFormats[i];
        desc.SampleDesc       = { 1, 0 };
        desc.Layout           = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        desc.Flags            = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        D3D12_CLEAR_VALUE clearVal = {};
        clearVal.Format   = GBuffer::RTFormats[i];
        clearVal.Color[3] = 1.0f;

        // 초기 상태: PIXEL_SHADER_RESOURCE
        // Execute() 진입 시 RENDER_TARGET으로 전환
        ThrowIfFailed(device->CreateCommittedResource(
            &heapProps, D3D12_HEAP_FLAG_NONE,
            &desc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            &clearVal, IID_PPV_ARGS(&m_gBuffer.Textures[i])));

        // RTV
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format        = GBuffer::RTFormats[i];
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        device->CreateRenderTargetView(
            m_gBuffer.Textures[i].Get(), &rtvDesc,
            m_gBuffer.RtvHeap.AllocateHandle());

        // SRV — 기존 슬롯(SRVIndices[i])에 덮어씀 (AllocateIndex 재호출 없음)
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format                  = GBuffer::RTFormats[i];
        srvDesc.ViewDimension           = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2D.MipLevels     = 1;
        device->CreateShaderResourceView(
            m_gBuffer.Textures[i].Get(), &srvDesc,
            m_srvHeap->GetCPUHandle(m_gBuffer.SRVIndices[i]));
    }
}

void GeometryPass::CreateDepthTarget(ID3D12Device* device, int width, int height)
{
    D3D12_RESOURCE_DESC depthDesc = {};
    depthDesc.Dimension        = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Width            = width;
    depthDesc.Height           = height;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels        = 1;
    depthDesc.Format           = DEPTH_FORMAT;
    depthDesc.SampleDesc       = { 1, 0 };
    depthDesc.Layout           = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthDesc.Flags            = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearVal = {};
    clearVal.Format                  = DEPTH_FORMAT;
    clearVal.DepthStencil.Depth      = 1.0f;
    clearVal.DepthStencil.Stencil    = 0;

    D3D12_HEAP_PROPERTIES heapProps = { D3D12_HEAP_TYPE_DEFAULT };

    ThrowIfFailed(device->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE,
        &depthDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &clearVal, IID_PPV_ARGS(&m_depth.Buffer)));

    m_depth.Heap.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
    device->CreateDepthStencilView(m_depth.Buffer.Get(), nullptr, m_depth.Heap.AllocateHandle());
}
