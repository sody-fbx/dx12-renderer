#pragma once

// ═══════════════════════════════════════════════════════════════════
//  IRenderPass.h
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"
#include "Pass/FrameResource.h"
#include "Scene/RenderItem.h"
#include <vector>

struct FrameContext
{
    ID3D12GraphicsCommandList*  CmdList      = nullptr;
    ID3D12Resource*             BackBuffer   = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE RTV          = {};
    UINT                        FrameIndex   = 0;
    int                         ScreenWidth  = 0;
    int                         ScreenHeight = 0;

    // CB 접근
    FrameResource* CurrentFrameResource = nullptr;

    // 렌더링 대상
    const std::vector<std::unique_ptr<RenderItem>>* RenderItems = nullptr;
};

class IRenderPass
{
public:
    virtual ~IRenderPass() = default;

    // 매 프레임 실행
    virtual void Execute(const FrameContext& ctx) = 0;

    // 윈도우 리사이즈 시
    virtual void OnResize(ID3D12Device* device, int width, int height) {}

    // ImGui 디버그 UI — 각 Pass가 자신의 파라미터를 노출
    virtual void OnDrawDebugUI() {}
};
