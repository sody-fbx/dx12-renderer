#pragma once

// ═══════════════════════════════════════════════════════════════════
//  GeometryPass.h — Deferred Rendering Geometry Pass
//  G-Buffer: [0] Albedo / [1] Normal / [2] WorldPos
// ═══════════════════════════════════════════════════════════════════

#include "Pass/PassUtil.h"
#include "Resource/RenderContext.h"

class GeometryPass : public IRenderPass
{
public:
    void Setup( ID3D12Device*        device
              , int                  width
              , int                  height
              , DescriptorHeap&      srvHeap
              , const RenderContext* renderCtx );

    void Execute(const FrameContext& ctx) override;
    void OnResize(ID3D12Device* device, int width, int height) override;
    void OnDrawDebugUI() override;

    const GBuffer& GetGBuffer() const { return m_gBuffer; }

private:
    void CreateGBufferTargets(ID3D12Device* device, int width, int height);
    void CreateDepthTarget(ID3D12Device* device, int width, int height);

    GBuffer      m_gBuffer;
    DepthTarget  m_depth;

    RootSignature m_rootSig;
    PipelineState m_pso;

    DescriptorHeap*      m_srvHeap  = nullptr;
    const RenderContext* m_renderCtx = nullptr;
};
