// ═══════════════════════════════════════════════════════════════════
//  LightingPass.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Pass/LightingPass.h"
#include "imgui.h"

void LightingPass::Setup( ID3D12Device*        device
                        , const ShadowMap*     shadowMap
                        , const GBuffer*       gBuffer
                        , const RenderContext* renderCtx )
{
    m_shadowMap = shadowMap;
    m_gBuffer   = gBuffer;
    m_renderCtx = renderCtx;

    m_rootSig.CreateWithLighting(device);
    m_pso.CreateLighting( device
                        , m_rootSig.Get()
                        , GetShaderPath(SHADERTYPE::LIGHTING)
                        , GetShaderPath(SHADERTYPE::LIGHTING)
                        , BACK_BUFFER_FORMAT );
}

void LightingPass::Execute(const FrameContext& ctx)
{
    auto* cmdList  = ctx.CmdList;
    auto* curFrame = ctx.CurrentFrameResource;

    // Viewport & Scissor
    D3D12_VIEWPORT vp = { 0, 0, (float)ctx.ScreenWidth, (float)ctx.ScreenHeight, 0, 1 };
    D3D12_RECT     sc = { 0, 0, ctx.ScreenWidth, ctx.ScreenHeight };
    cmdList->RSSetViewports(1, &vp);
    cmdList->RSSetScissorRects(1, &sc);

    // BackBuffer 클리어 후 바인딩 (Depth 비활성화)
    auto RTV = ctx.RTV;
    const float clearColor[] = { 0.05f, 0.05f, 0.08f, 1.0f };
    cmdList->ClearRenderTargetView(RTV, clearColor, 0, nullptr);
    cmdList->OMSetRenderTargets(1, &RTV, FALSE, nullptr);

    // Root Signature & PSO
    cmdList->SetGraphicsRootSignature(m_rootSig.Get());
    cmdList->SetPipelineState(m_pso.Get());

    // [0] PassCB (b0)
    cmdList->SetGraphicsRootConstantBufferView(0, curFrame->PassCB->GetElementGPUAddress(0));

    const DescriptorHeap* srvHeap = m_renderCtx ? m_renderCtx->SrvHeap : nullptr;

    if (srvHeap)
    {
        ID3D12DescriptorHeap* heaps[] = { srvHeap->Get() };
        cmdList->SetDescriptorHeaps(1, heaps);

        // [1] Shadow Map SRV (t0)
        if (m_shadowMap && m_shadowMap->IsValid())
            cmdList->SetGraphicsRootDescriptorTable(1, srvHeap->GetGPUHandle(m_shadowMap->SRVIndex));

        // [2] G-Buffer SRVs (t1~t3) — SRVIndices[0]부터 연속 3개
        if (m_gBuffer && m_gBuffer->IsValid())
            cmdList->SetGraphicsRootDescriptorTable(2, srvHeap->GetGPUHandle(m_gBuffer->SRVIndices[0]));
    }

    // 버텍스 버퍼 없이 SV_VertexID 사용
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->DrawInstanced(3, 1, 0, 0);
}

void LightingPass::OnDrawDebugUI()
{
    if (ImGui::CollapsingHeader("Lighting Pass (Deferred)"))
    {
        ImGui::Text("Mode    : Fullscreen Triangle");
        ImGui::Text("Inputs  : Shadow Map + G-Buffer x3");
        if (m_shadowMap)
            ImGui::Text("Shadow SRV : %u", m_shadowMap->SRVIndex);
        if (m_gBuffer)
            ImGui::Text("GBuf SRV[0]: %u", m_gBuffer->SRVIndices[0]);
    }
}
