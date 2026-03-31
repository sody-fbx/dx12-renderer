#pragma once

// ═══════════════════════════════════════════════════════════════════
//  LightingPass.h — Deferred Rendering Lighting Pass
// ═══════════════════════════════════════════════════════════════════

#include "Pass/PassUtil.h"
#include "Resource/RenderContext.h"

class LightingPass : public IRenderPass
{
public:
    void Setup( ID3D12Device*        device
              , const ShadowMap*     shadowMap
              , const GBuffer*       gBuffer
              , const RenderContext* renderCtx );

    void Execute(const FrameContext& ctx) override;
    void OnResize(ID3D12Device* device, int width, int height) override {}
    void OnDrawDebugUI() override;

private:
    RootSignature m_rootSig;
    PipelineState m_pso;

    const ShadowMap*     m_shadowMap = nullptr;
    const GBuffer*       m_gBuffer   = nullptr;
    const RenderContext* m_renderCtx = nullptr;
};
