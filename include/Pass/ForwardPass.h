#pragma once

// ═══════════════════════════════════════════════════════════════════
//  ForwardPass.h
// ═══════════════════════════════════════════════════════════════════

#include "Pass/PassUtil.h"

#include "Resource/RenderContext.h"

class ForwardPass : public IRenderPass
{
public:
    void Setup(ID3D12Device* device, int width, int height,
               const ShadowMap* shadowMap, const RenderContext* renderCtx);
    void Execute(const FrameContext& ctx) override;
    void OnResize(ID3D12Device* device, int width, int height) override;

private:
    void CreateDepthTarget(ID3D12Device* device, int width, int height);

    // DSV
    DepthTarget m_depth;

    // PSO
    // TODO : 추후 Map으로 만들어, 프로젝트 실행 시 Pipeline을 모두 미리 만들어두고
    // Object에서 골라서 사용할 수 있도록 변경 예정
    PipelineSet m_pipelineSet;

    // Link Data
    const ShadowMap* m_shadowMap = nullptr;
    const RenderContext* m_renderCtx = nullptr;
};
