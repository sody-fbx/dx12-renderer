// ═══════════════════════════════════════════════════════════════════
//  ForwardPass.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Pass/ForwardPass.h"

void ForwardPass::Setup(ID3D12Device* device, int width, int height)
{
    m_pipelineSet.Create(device, ROOT_SIGNATURE_TYPE_CBV, SHADERTYPE::DEFAULT);
    m_depth.Create(device, width, height);
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
    // Depth 재생성
    m_depth.Resize(device, width, height);
}
