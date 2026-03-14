// ═══════════════════════════════════════════════════════════════════
//  Renderer.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Core/Renderer.h"
#include "Resource/GeometryGenerator.h"

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
    auto triangleData = GeometryGenerator::CreateTriangle();

    m_commandList.Reset(0, nullptr);
    auto cmdList = m_commandList.Get();

    m_mesh = std::make_unique<Mesh>();
    m_mesh->Create( m_device.GetDevice()
                 , cmdList
                 , triangleData.Vertices.data(), (UINT)triangleData.Vertices.size(), sizeof(VertexCol)
                 , triangleData.Indices.data(), (UINT)triangleData.Indices.size(), DXGI_FORMAT_R32_UINT
                 );

    m_commandList.Close();
    m_commandQueue.ExecuteCommandList(cmdList);
    m_commandQueue.Flush();
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
    auto vbv = m_mesh->VertexBufferView();
    auto ibv = m_mesh->IndexBufferView();
    cmdList->IASetVertexBuffers(0, 1, &vbv);
    cmdList->IASetIndexBuffer(&ibv);
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->DrawIndexedInstanced(m_mesh->GetIndexCount(), 1, 0, 0, 0);

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
