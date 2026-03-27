#pragma once

// ═══════════════════════════════════════════════════════════════════
//  ShadowPass.h
// ═══════════════════════════════════════════════════════════════════

#include "Pass/PassUtil.h"

class ShadowPass : public IRenderPass
{
public:
    void Setup(ID3D12Device* device, int width, int height, DescriptorHeap& srvAllocator);
    void Execute(const FrameContext& ctx) override;
    void OnDrawDebugUI() override;

public: // Getter
    const ShadowMap& GetShadowMap() const;

private:
    void CreateShadowMap(ID3D12Device* device, DescriptorHeap& allocator);

    // Shadow Map
    ShadowMap m_shadowMap;

    // Pipeline
    PipelineSet m_pipelineSet;
};
