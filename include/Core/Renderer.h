#pragma once

// ═══════════════════════════════════════════════════════════════════
//  Renderer.h
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DDevice.h"
#include "Core/SwapChain.h"
#include "Core/CommandQueue.h"
#include "Core/CommandList.h"
#include "Core/RootSignature.h"
#include "Core/PipelineState.h"

#include "Resource/Mesh.h"
#include "Resource/FrameResource.h"
#include "Resource/GeometryGenerator.h"

class Renderer
{
public:
    void Initialize(HWND hwnd, int width, int height);
    void Shutdown();

    void Render();
    void OnResize(int width, int height);

private:
    void BeginFrame();
    void EndFrame();

    void BuildGeometry();
    void UpdateConstantBuffers();

    // DX12 Core Module
    D3DDevice      m_device;
    SwapChain      m_swapChain;
    CommandQueue   m_commandQueue;
    CommandList    m_commandList;
    RootSignature  m_rootSignature;
    PipelineState  m_pso;

    // Mesh
    std::unique_ptr<Mesh>   m_mesh;

    // FrameResource
    std::array<FrameResource, FRAME_BUFFER_COUNT>   m_frameRes;

    // 프레임 동기화
    std::array<UINT64, FRAME_BUFFER_COUNT> m_frameFenceValues = {};

    int m_width  = 0;
    int m_height = 0;
};
