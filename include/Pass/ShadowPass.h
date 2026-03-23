#pragma once

// ═══════════════════════════════════════════════════════════════════
//  ShadowPass.h
// ═══════════════════════════════════════════════════════════════════

#include "Pass/PassUtil.h"

class ShadowPass : public IRenderPass
{
public:
    void Setup(ID3D12Device* device, int width, int height) override;
    void Execute(const FrameContext& ctx) override;

public: // Getter
    const ShadowMap& GetShadowMap() const { return m_shadowMap; }

private:
    void CreateShadowMap(ID3D12Device* device);

    // Shadow Map
    ShadowMap m_shadowMap;

    // Pipeline
    PipelineSet m_pipelineSet;
};
