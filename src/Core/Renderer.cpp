// ═══════════════════════════════════════════════════════════════════
//  Renderer.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Core/Renderer.h"

struct Vertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
};

void Renderer::Initialize(HWND hwnd, int width, int height)
{
    m_width  = width;
    m_height = height;

    // 초기화 순서
    // Device -> CommandQueue -> SwapChain -> CommandList -> RootSig -> PSO
    m_device.Initialize();

    m_commandQueue.Initialize(m_device.GetDevice());

    m_swapChain.Initialize( m_device.GetFactory()
                          , m_commandQueue.GetQueue()
                          , m_device.GetDevice()
                          , hwnd, width, height );

    m_commandList.Initialize(m_device.GetDevice());

    m_rootSignature.CreateEmpty(m_device.GetDevice());

    m_pso.Create( m_device.GetDevice()
                , m_rootSignature.Get()
                , L"src/Shaders/Triangle.hlsl"
                , L"src/Shaders/Triangle.hlsl"
                , m_swapChain.GetBackBufferFormat()
                , m_swapChain.GetDepthFormat() );

    BuildTriangleGeometry();
}

void Renderer::BuildTriangleGeometry()
{
    // ── TEST : 삼각형 그리기 ──
    Vertex vertices[] =
    {
        { {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },  // 상단 (빨강)
        { {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },  // 우하 (초록)
        { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },  // 좌하 (파랑)
    };
    const UINT vbSize = sizeof(vertices);

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width            = vbSize;
    bufferDesc.Height           = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels        = 1;
    bufferDesc.SampleDesc       = { 1, 0 };
    bufferDesc.Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    ThrowIfFailed(
        m_device.GetDevice()->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_vertexBuffer)
        )
    );

    // 정점 데이터 복사
    void* mappedData = nullptr;
    ThrowIfFailed(m_vertexBuffer->Map(0, nullptr, &mappedData));
    memcpy(mappedData, vertices, vbSize);
    m_vertexBuffer->Unmap(0, nullptr);

    // Vertex Buffer View 생성
    m_vbView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vbView.SizeInBytes    = vbSize;
    m_vbView.StrideInBytes  = sizeof(Vertex);
}

void Renderer::BeginFrame()
{
    UINT frameIndex = m_swapChain.CurrentBackBufferIndex();

    // Fence 대기
    m_commandQueue.WaitForFenceValue(m_frameFenceValues[frameIndex]);

    // CommandList 리셋
    m_commandList.Reset(frameIndex, m_pso.Get());
    auto* cmdList = m_commandList.Get();

    // Barrier: PRESENT -> RENDER_TARGET
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource   = m_swapChain.CurrentBackBuffer();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    cmdList->ResourceBarrier(1, &barrier);
}

void Renderer::Render()
{
    BeginFrame();

    auto* cmdList = m_commandList.Get();

    // Viewport & Scissor
    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width    = static_cast<float>(m_width);
    viewport.Height   = static_cast<float>(m_height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    cmdList->RSSetViewports(1, &viewport);

    D3D12_RECT scissor = { 0, 0, m_width, m_height };
    cmdList->RSSetScissorRects(1, &scissor);

    // Clear RTV & DSV
    const float clearColor[] = { 0.05f, 0.05f, 0.15f, 1.0f };
    auto rtv = m_swapChain.CurrentRTV();
    auto dsv = m_swapChain.DSV();
    cmdList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    cmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // 렌더타겟 바인딩
    cmdList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

    // PSO, Root Signature 바인딩
    cmdList->SetPipelineState(m_pso.Get());
    cmdList->SetGraphicsRootSignature(m_rootSignature.Get());

    // ── TEST : 삼각형 그리기 ──
    cmdList->IASetVertexBuffers(0, 1, &m_vbView);
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->DrawInstanced(3, 1, 0, 0);
    EndFrame();
}

void Renderer::EndFrame()
{
    auto* cmdList = m_commandList.Get();

    // Resource Barrier: RENDER_TARGET -> PRESENT
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource   = m_swapChain.CurrentBackBuffer();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    cmdList->ResourceBarrier(1, &barrier);

    m_commandList.Close();
    m_commandQueue.ExecuteCommandList(m_commandList.Get());

    // Present
    m_swapChain.Present(true);

    // Fence Signal
    UINT frameIndex = m_swapChain.CurrentBackBufferIndex();
    m_frameFenceValues[frameIndex] = m_commandQueue.Signal();
}

void Renderer::OnResize(int width, int height)
{
    if (width == 0 || height == 0) return;

    m_width  = width;
    m_height = height;

    // GPU가 모든 Back Buffer 사용을 끝낼 때까지 대기
    m_commandQueue.Flush();

    m_swapChain.Resize(m_device.GetDevice(), width, height);
}

void Renderer::Shutdown()
{
    m_commandQueue.Shutdown();
}
