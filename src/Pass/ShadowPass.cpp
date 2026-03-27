// ═══════════════════════════════════════════════════════════════════
//  ShadowPass.cpp — Shadow Map 생성 Pass
// ═══════════════════════════════════════════════════════════════════

#include "Pass/ShadowPass.h"
#include "imgui.h"

void ShadowPass::Setup(ID3D12Device* device, int width, int height, DescriptorHeap& srvAllocator)
{
    m_pipelineSet.Create(device, ROOT_SIGNATURE_TYPE_CBV, SHADERTYPE::SHADOW);
    CreateShadowMap(device, srvAllocator);
}

void ShadowPass::Execute(const FrameContext& ctx)
{
    auto* cmdList  = ctx.CmdList;
    auto* curFrame = ctx.CurrentFrameResource;

    // Barrier: PIXEL_SHADER_RESOURCE -> DEPTH_WRITE
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource   = m_shadowMap.Texture.Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    cmdList->ResourceBarrier(1, &barrier);

    // Viewport = Shadow Map 크기
    D3D12_VIEWPORT vp = { 0, 0
                        , static_cast<float>(m_shadowMap.Size)
                        , static_cast<float>(m_shadowMap.Size)
                        , 0, 1 };
    D3D12_RECT sc = { 0, 0, (LONG)m_shadowMap.Size, (LONG)m_shadowMap.Size };
    cmdList->RSSetViewports(1, &vp);
    cmdList->RSSetScissorRects(1, &sc);

    // DSV 바인딩
    auto dsvHandle = m_shadowMap.Depth.Heap.GetCPUHandle(0);
    cmdList->OMSetRenderTargets(0, nullptr, FALSE, &dsvHandle);
    cmdList->ClearDepthStencilView( dsvHandle
                                  , D3D12_CLEAR_FLAG_DEPTH
                                  , 1.0f, 0, 0, nullptr);

    // RootSignature & PSO
    cmdList->SetPipelineState(m_pipelineSet.Pso.Get());
    cmdList->SetGraphicsRootSignature(m_pipelineSet.RootSignature.Get());

    // ShadowCB 바인딩
    cmdList->SetGraphicsRootConstantBufferView(1, curFrame->ShadowCB->GetElementGPUAddress(0));

    // RenderItem(Mesh) for light Draw
    for (auto& item : *ctx.RenderItems)
    {
        auto vertexView = item->MeshRef->VertexBufferView();
        auto indexView  = item->MeshRef->IndexBufferView();
        cmdList->IASetVertexBuffers(0, 1, &vertexView);
        cmdList->IASetIndexBuffer(&indexView);
        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        cmdList->SetGraphicsRootConstantBufferView(0, curFrame->ObjectCB->GetElementGPUAddress(item->ObjCBIndex));

        cmdList->DrawIndexedInstanced(item->MeshRef->GetIndexCount(), 1, 0, 0, 0);
    }

    // Barrier: DEPTH_WRITE -> PIXEL_SHADER_RESOURCE
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    cmdList->ResourceBarrier(1, &barrier);
}

void ShadowPass::OnDrawDebugUI()
{
    if (ImGui::CollapsingHeader("Shadow Pass"))
    {
        ImGui::Text("Shadow Map: %dx%d", m_shadowMap.Size, m_shadowMap.Size);
        ImGui::Text("SRV Index : %u",    m_shadowMap.SRVIndex);
    }
}

void ShadowPass::CreateShadowMap(ID3D12Device* device, DescriptorHeap& allocator)
{
	// Depth/Stencil Buffer
	// Shadow Map 텍스처는 Typeless(DSV는 D32, SRV는 R32로 해석)
	D3D12_RESOURCE_DESC texDesc     = {};
	texDesc.Dimension               = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Width                   = m_shadowMap.Size;
	texDesc.Height                  = m_shadowMap.Size;
	texDesc.DepthOrArraySize        = 1;
	texDesc.MipLevels               = 1;
	texDesc.Format                  = DXGI_FORMAT_R32_TYPELESS;
	texDesc.SampleDesc              = { 1, 0 };
	texDesc.Flags                   = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearValue    = {};
	clearValue.Format               = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth   = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	ThrowIfFailed(device->CreateCommittedResource( &heapProps, D3D12_HEAP_FLAG_NONE
		                                         , &texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		                                         , &clearValue, IID_PPV_ARGS(&m_shadowMap.Texture)));

	// DSV (Depth 쓰기 전용)
    m_shadowMap.Depth.Heap.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView( m_shadowMap.Texture.Get()
		                          , &dsvDesc
		                          , m_shadowMap.Depth.Heap.AllocateHandle());

	// SRV (ForwardPass에서 읽기용)
    m_shadowMap.SRVIndex = allocator.AllocateIndex();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView( m_shadowMap.Texture.Get()
                                    , &srvDesc
                                    , allocator.GetCPUHandle(m_shadowMap.SRVIndex));
}

const ShadowMap& ShadowPass::GetShadowMap() const
{
    return m_shadowMap;
}
