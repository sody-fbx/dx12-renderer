// ═══════════════════════════════════════════════════════════════════
//  Renderer.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Core/Renderer.h"

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

    //m_rootSignature.CreateEmpty(m_device.GetDevice());
    m_rootSignature.CreateWithCBV(m_device.GetDevice());

    //m_pso.Create( m_device.GetDevice()
    //            , m_rootSignature.Get()
    //            , L"src/Shaders/Triangle.hlsl"
    //            , L"src/Shaders/Triangle.hlsl"
    //            , m_swapChain.GetBackBufferFormat()
    //            , m_swapChain.GetDepthFormat() );
    m_pso.Create( m_device.GetDevice()
                , m_rootSignature.Get()
                , L"src/Shaders/Default.hlsl"
                , L"src/Shaders/Default.hlsl"
                , m_swapChain.GetBackBufferFormat()
                , m_swapChain.GetDepthFormat() );

    for (auto& fr : m_frameRes)
        fr.Initialize(m_device.GetDevice(), 1);     // 오브젝트 1개

    m_camera.Initialize(5.0f, 45.0f, (float)m_width / m_height, 0.1f, 100.0f);

    BuildGeometry();
}

void Renderer::BuildGeometry()
{
    auto meshData = GeometryGenerator::CreateBox(1.0f, 1.0f, 1.0f);

    m_commandList.Reset(0, nullptr);
    auto cmdList = m_commandList.Get();

    m_mesh = std::make_unique<Mesh>();
    m_mesh->Create( m_device.GetDevice()
                 , cmdList
                 , meshData.Vertices.data(), (UINT)meshData.Vertices.size(), sizeof(VertexTex)
                 , meshData.Indices.data(), (UINT)meshData.Indices.size(), DXGI_FORMAT_R32_UINT
                 );

    m_commandList.Close();
    m_commandQueue.ExecuteCommandList(cmdList);
    m_commandQueue.Flush();
}

void Renderer::Update(float mouseDx, float mouseDy, float wheelDelta)
{
    if (mouseDx != 0.0f || mouseDy != 0.0f)
        m_camera.Rotate(mouseDx, mouseDy);

    if (wheelDelta != 0.0f)
        m_camera.Zoom(wheelDelta);
}

void Renderer::UpdateConstantBuffers()
{
    UINT frameIndex = m_swapChain.CurrentBackBufferIndex();
    auto& curFrame = m_frameRes[frameIndex];

    // PassCB 갱신: View / Proj
    XMMATRIX view = m_camera.GetViewMatrix();
    XMMATRIX proj = m_camera.GetProjMatrix();
    XMMATRIX viewProj = view * proj;

    PassConstants passData;
    XMStoreFloat4x4(&passData.View, XMMatrixTranspose(view));
    XMStoreFloat4x4(&passData.Proj, XMMatrixTranspose(proj));
    XMStoreFloat4x4(&passData.ViewProj, XMMatrixTranspose(viewProj));
    passData.EyePos = m_camera.GetPosition();

    curFrame.PassCB->CopyData(0, passData);

    // ObjectCB 갱신: World
    ObjectConstants objData;
    XMStoreFloat4x4(&objData.World, XMMatrixTranspose(XMMatrixIdentity()));

    curFrame.ObjectCB->CopyData(0, objData);
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

    // CB Update
    UpdateConstantBuffers();

    auto* cmdList = m_commandList.Get();
    UINT frameIndex = m_swapChain.CurrentBackBufferIndex();
    auto& curFrame = m_frameRes[frameIndex];

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

    // Root Signature 바인딩
    cmdList->SetGraphicsRootSignature(m_rootSignature.Get());

    // CB 바인딩
    cmdList->SetGraphicsRootConstantBufferView(0, curFrame.ObjectCB->GetElementGPUAddress(0));
    cmdList->SetGraphicsRootConstantBufferView(1, curFrame.PassCB->GetElementGPUAddress(0));

    // Mesh Draw
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
    m_frameFenceValues.fill(0);

    m_camera.SetAspectRatio((float)width / height);
}

void Renderer::Shutdown()
{
    m_commandQueue.Shutdown();
}
