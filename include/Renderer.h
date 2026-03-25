#pragma once

// ═══════════════════════════════════════════════════════════════════
//  Renderer.h
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DDevice.h"
#include "Core/SwapChain.h"
#include "Core/CommandQueue.h"
#include "Core/CommandList.h"

#include "Pass/IRenderPass.h"
#include "Pass/ForwardPass.h"
#include "Pass/ShadowPass.h"
#include "Pass/ImGuiPass.h"

#include "imgui_impl_win32.h"

#include "Resource/Mesh.h"
#include "Resource/FrameResource.h"

#include "Scene/Scene.h"

class Renderer
{
public:
    void Initialize(HWND hwnd, int width, int height);
    void Shutdown();

    void Update(float mouseDx, float mouseDy, float wheelDelta);
    void Render();
    void OnResize(int width, int height);

private:
    void BeginFrame();
    void EndFrame();

    void BuildPasses();
    void UpdateConstantBuffers();
    void DrawImGui();

private:
    // DX12 Core Module
    D3DDevice      m_device;
    SwapChain      m_swapChain;
    CommandQueue   m_commandQueue;
    CommandList    m_commandList;

    // Pass
    std::vector<IRenderPass*>    m_passes;
    std::unique_ptr<ForwardPass> m_forwardPass;
    std::unique_ptr<ShadowPass>  m_shadowPass;
    std::unique_ptr<ImGuiPass>   m_imGuiPass;

    // Scene
    Scene m_scene;

    // Mesh
    std::unordered_map<std::string, std::unique_ptr<Mesh>>   m_meshes;

    // FrameResource
    std::array<FrameResource, FRAME_BUFFER_COUNT>   m_frameRes;

    // 프레임 동기화
    std::array<UINT64, FRAME_BUFFER_COUNT> m_frameFenceValues = {};

    HWND m_hwnd;
    int m_width  = 0;
    int m_height = 0;
};
