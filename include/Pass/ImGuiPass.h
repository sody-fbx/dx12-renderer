#pragma once

// ═══════════════════════════════════════════════════════════════════
//  ImGuiPass.h — ImGui UI 오버레이 Pass
//  ImGui 컨텍스트, Win32/DX12 백엔드, Font SRV Heap 관리.
// ═══════════════════════════════════════════════════════════════════

#include "Pass/PassUtil.h"

class ImGuiPass : public IRenderPass
{
public:
    void Setup(ID3D12Device* device, int width, int height);
    void Execute(const FrameContext& ctx) override;
    void OnResize(ID3D12Device* device, int width, int height) override;
    void Shutdown();

    // ImGui 백엔드 초기화
    void InitBackend( ID3D12Device* device
                    , ID3D12CommandQueue* commandQueue
                    , HWND hwnd);

    void BeginFrame();

private:
    DescriptorHeap m_srvHeap;   // Font 텍스처 SRV
    bool m_initialized = false;
};
