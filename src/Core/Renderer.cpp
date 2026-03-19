// ═══════════════════════════════════════════════════════════════════
//  Renderer.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Core/Renderer.h"

void Renderer::Initialize(HWND hwnd, int width, int height)
{
    m_width  = width;
    m_height = height;

    // 초기화 순서
    // Device -> CommandQueue -> SwapChain -> CommandList
    m_device.Initialize();
    m_commandQueue.Initialize(m_device.GetDevice());
    m_swapChain.Initialize( m_device.GetFactory()
                          , m_commandQueue.GetQueue()
                          , m_device.GetDevice()
                          , hwnd, width, height );
    m_commandList.Initialize(m_device.GetDevice());

    m_camera.Initialize(5.0f, 45.0f, (float)m_width / m_height, 0.1f, 100.0f);

    BuildGeometry();
    BuildRenderItem();

    for (auto& fr : m_frameRes)
        fr.Initialize(m_device.GetDevice(), (UINT)m_renderItems.size());

    BuildPasses();
}

void Renderer::BuildGeometry()
{
    m_commandList.Reset(0, nullptr);
    auto cmdList = m_commandList.Get();

    // Box
    {
        auto data = GeometryGenerator::CreateBox(1.0f, 1.0f, 1.0f);
        auto mesh = std::make_unique<Mesh>();
        mesh->Create(
            m_device.GetDevice(), cmdList,
            data.Vertices.data(), (UINT)data.Vertices.size(), sizeof(VertexTex),
            data.Indices.data(), (UINT)data.Indices.size(), DXGI_FORMAT_R32_UINT
        );
        mesh->Name = "Box";
        m_meshes["Box"] = std::move(mesh);
    }

    // Sphere
    {
        auto data = GeometryGenerator::CreateSphere(0.5f, 20, 20);
        auto mesh = std::make_unique<Mesh>();
        mesh->Create(
            m_device.GetDevice(), cmdList,
            data.Vertices.data(), (UINT)data.Vertices.size(), sizeof(VertexTex),
            data.Indices.data(), (UINT)data.Indices.size(), DXGI_FORMAT_R32_UINT
        );
        mesh->Name = "Sphere";
        m_meshes["Sphere"] = std::move(mesh);
    }

    // Cylinder
    {
        auto data = GeometryGenerator::CreateCylinder(0.4f, 0.4f, 1.5f, 20, 5);
        auto mesh = std::make_unique<Mesh>();
        mesh->Create(
            m_device.GetDevice(), cmdList,
            data.Vertices.data(), (UINT)data.Vertices.size(), sizeof(VertexTex),
            data.Indices.data(), (UINT)data.Indices.size(), DXGI_FORMAT_R32_UINT
        );
        mesh->Name = "Cylinder";
        m_meshes["Cylinder"] = std::move(mesh);
    }

    // Grid
    {
        auto data = GeometryGenerator::CreateGrid(10.0f, 10.0f, 20, 20);
        auto mesh = std::make_unique<Mesh>();
        mesh->Create(
            m_device.GetDevice(), cmdList,
            data.Vertices.data(), (UINT)data.Vertices.size(), sizeof(VertexTex),
            data.Indices.data(), (UINT)data.Indices.size(), DXGI_FORMAT_R32_UINT
        );
        mesh->Name = "Grid";
        m_meshes["Grid"] = std::move(mesh);
    }

    m_commandList.Close();
    m_commandQueue.ExecuteCommandList(cmdList);
    m_commandQueue.Flush();
}

void Renderer::BuildRenderItem()
{
    UINT cbIndex = 0;

    // Grid (바닥, y=0)
    {
        auto item = std::make_unique<RenderItem>();
        item->MeshRef = m_meshes["Grid"].get();
        item->ObjCBIndex = cbIndex++;
        // World = Identity (원점에 수평으로 깔림)
        m_renderItems.push_back(std::move(item));
    }

    // Box (원점 위, y=0.5)
    {
        auto item = std::make_unique<RenderItem>();
        item->MeshRef = m_meshes["Box"].get();
        item->ObjCBIndex = cbIndex++;
        XMStoreFloat4x4(&item->World,
            XMMatrixTranslation(0.0f, 0.5f, 0.0f));
        m_renderItems.push_back(std::move(item));
    }

    // Sphere (오른쪽, x=3)
    {
        auto item = std::make_unique<RenderItem>();
        item->MeshRef = m_meshes["Sphere"].get();
        item->ObjCBIndex = cbIndex++;
        XMStoreFloat4x4(&item->World,
            XMMatrixTranslation(3.0f, 0.5f, 0.0f));
        m_renderItems.push_back(std::move(item));
    }

    // Cylinder (왼쪽, x=-3)
    {
        auto item = std::make_unique<RenderItem>();
        item->MeshRef = m_meshes["Cylinder"].get();
        item->ObjCBIndex = cbIndex++;
        XMStoreFloat4x4(&item->World,
            XMMatrixTranslation(-3.0f, 0.75f, 0.0f));
        m_renderItems.push_back(std::move(item));
    }
}

void Renderer::BuildPasses()
{
    // ForwardPass
    m_forwardPass = std::make_unique<ForwardPass>();
    m_forwardPass->Setup(m_device.GetDevice(), m_width, m_height);

    m_passes.clear();
    m_passes.push_back(m_forwardPass.get());
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
    
    // ObjectCB (RenderItem별 갱신)
    for (auto& item : m_renderItems)
    {
        // NumFramesDirty > 0이면 이 오브젝트의 CB를 갱신해야 함.
        // World 행렬이 바뀔 때 NumFramesDirty = FRAME_BUFFER_COUNT로 세팅.
        // 트리플 버퍼링이라 3개 FrameResource 모두 반영되어야 하므로
        // 3프레임 동안 갱신 후 0이 되면 스킵.
        if (item->NumFramesDirty > 0)
        {
            ObjectConstants objData;
            XMStoreFloat4x4(&objData.World, XMMatrixTranspose(XMLoadFloat4x4(&item->World)));

            curFrame.ObjectCB->CopyData(item->ObjCBIndex, objData);
            item->NumFramesDirty--;
        }
    }
}

void Renderer::BeginFrame()
{
    UINT frameIndex = m_swapChain.CurrentBackBufferIndex();

    // Fence 대기
    m_commandQueue.WaitForFenceValue(m_frameFenceValues[frameIndex]);

    // CommandList 리셋
    m_commandList.Reset(frameIndex, nullptr);
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

    // FrameContext 구성
    UINT frameIndex = m_swapChain.CurrentBackBufferIndex();
    FrameContext ctx = {};
    ctx.CmdList = m_commandList.Get();
    ctx.BackBuffer = m_swapChain.CurrentBackBuffer();
    ctx.RTV = m_swapChain.CurrentRTV();
    ctx.FrameIndex = frameIndex;
    ctx.ScreenWidth = m_width;
    ctx.ScreenHeight = m_height;
    ctx.CurrentFrameResource = &m_frameRes[frameIndex];
    ctx.RenderItems = &m_renderItems;

    // Pass 실행
    for (auto* pass : m_passes)
    {
        pass->Execute(ctx);
    }

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

    for (auto* pass : m_passes)
        pass->OnResize(m_device.GetDevice(), width, height);
}

void Renderer::Shutdown()
{
    m_commandQueue.Shutdown();
}
