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

#include "Resource/Mesh.h"
#include "Resource/FrameResource.h"
#include "Resource/GeometryGenerator.h"

#include "Scene/Camera.h"
#include "Scene/RenderItem.h"

class Renderer
{
public:
    void Initialize(HWND hwnd, int width, int height);
    void Shutdown();

    void Update(float mouseDx, float mouseDy, float wheelDelta);
    void Render();
    void OnResize(int width, int height);

    Camera& GetCamera() { return m_camera; }

private:
    void BeginFrame();
    void EndFrame();

    void BuildGeometry();
    void BuildRenderItem();
    void BuildPasses();
    void UpdateConstantBuffers();

private:
    // DX12 Core Module
    D3DDevice      m_device;
    SwapChain      m_swapChain;
    CommandQueue   m_commandQueue;
    CommandList    m_commandList;

    // Pass
    std::vector<IRenderPass*>    m_passes;
    std::unique_ptr<ForwardPass> m_forwardPass;

    // Mesh
    std::unordered_map<std::string, std::unique_ptr<Mesh>>   m_meshes;

    // Scene Object
    std::vector<std::unique_ptr<RenderItem>> m_renderItems;

    // FrameResource
    std::array<FrameResource, FRAME_BUFFER_COUNT>   m_frameRes;

    // Scene
    Camera m_camera;

    // 프레임 동기화
    std::array<UINT64, FRAME_BUFFER_COUNT> m_frameFenceValues = {};

    int m_width  = 0;
    int m_height = 0;
};
