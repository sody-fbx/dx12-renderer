#pragma once

// ═══════════════════════════════════════════════════════════════════
//  Renderer.h
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DDevice.h"
#include "Core/SwapChain.h"
#include "Core/CommandQueue.h"
#include "Core/CommandList.h"
#include "Core/DescriptorHeap.h"

#include "Pass/IRenderPass.h"
#include "Pass/ForwardPass.h"
#include "Pass/ShadowPass.h"
#include "Pass/ImGuiPass.h"

#include "imgui_impl_win32.h"

#include "Pass/FrameResource.h"
#include "Resource/MeshManager.h"
#include "Resource/TextureManager.h"

#include "Scene/SceneManager.h"

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

    void BuildRenderContext();
    void BuildPasses();
    void UpdateConstantBuffers();
    void DrawImGui();

private:
    // DX12 Core
    D3DDevice      m_device;
    SwapChain      m_swapChain;
    CommandQueue   m_commandQueue;
    CommandList    m_commandList;

    // 공유 SRV Heap + Resource Manager
    DescriptorHeap m_srvHeap;
    MeshManager    m_meshManager;
    TextureManager m_texManager;
    RenderContext  m_renderCtx;

    // Pass
    std::vector<IRenderPass*>    m_passes;
    std::unique_ptr<ForwardPass> m_forwardPass;
    std::unique_ptr<ShadowPass>  m_shadowPass;
    std::unique_ptr<ImGuiPass>   m_imGuiPass;

    // Scene
    SceneManager m_sceneManager;

    // FrameResource
    std::array<FrameResource, FRAME_BUFFER_COUNT> m_frameRes;
    std::array<UINT64, FRAME_BUFFER_COUNT>        m_frameFenceValues = {};

    HWND m_hwnd;
    int  m_width  = 0;
    int  m_height = 0;
};
