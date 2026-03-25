// ═══════════════════════════════════════════════════════════════════
//  Renderer.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Renderer.h"

void Renderer::Initialize(HWND hwnd, int width, int height)
{
    m_hwnd = hwnd;
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

    // Scene 초기화
    Camera mainCam;
    mainCam.Initialize(5.0f, 45.0f, (float)m_width / m_height, 0.1f, 100.0f);
    m_scene.SetCamera(mainCam);

    m_scene.GetDirLight().Direction = { 0.5f, -1.0f, 0.3f };
    m_scene.GetDirLight().Color = { 1.0f, 1.0f, 0.9f };
    m_scene.GetDirLight().Intensity = 1.0f;

    m_commandList.Reset(0, nullptr);
    m_scene.Initialize(m_device.GetDevice(), m_commandList.Get());
    m_commandList.Close();
    m_commandQueue.ExecuteCommandList(m_commandList.Get());
    m_commandQueue.Flush();

    for (auto& fr : m_frameRes)
        fr.Initialize(m_device.GetDevice(), m_scene.GetObjectCount());

    BuildPasses();
}

void Renderer::BuildPasses()
{
    // ShadowPass
    m_shadowPass = std::make_unique<ShadowPass>();
    m_shadowPass->Setup(m_device.GetDevice(), m_width, m_height);

    // ForwardPass
    m_forwardPass = std::make_unique<ForwardPass>();
    m_forwardPass->Setup(m_device.GetDevice(), m_width, m_height);
    m_forwardPass->SetShadowMap(&m_shadowPass->GetShadowMap());

    // ImGuiPass
    m_imGuiPass = std::make_unique<ImGuiPass>();
    m_imGuiPass->Setup(m_device.GetDevice(), m_width, m_height);
    m_imGuiPass->InitBackend(m_device.GetDevice(), m_commandQueue.GetQueue(), m_hwnd);

    m_passes.clear();
    m_passes.push_back(m_shadowPass.get());
    m_passes.push_back(m_forwardPass.get());
    m_passes.push_back(m_imGuiPass.get());
}

void Renderer::Update(float mouseDx, float mouseDy, float wheelDelta)
{
    // ImGui가 마우스를 캡처 중이면 Scene 카메라 조작 안 함
    if (ImGui::GetIO().WantCaptureMouse)
        return;

    if (mouseDx != 0.0f || mouseDy != 0.0f)
        m_scene.GetCamera().Rotate(mouseDx, mouseDy);

    if (wheelDelta != 0.0f)
        m_scene.GetCamera().Zoom(wheelDelta);
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
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_swapChain.CurrentBackBuffer();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    cmdList->ResourceBarrier(1, &barrier);
}

void Renderer::Render()
{
    BeginFrame();

    // CB Update
    UpdateConstantBuffers();

    // Pass 순회 전에 ImGui 프레임 시작
    m_imGuiPass->BeginFrame();

    // ImGui 위젯 정의
    DrawImGui();

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
    ctx.RenderItems = &m_scene.GetRenderItems();

    // Pass 실행
    for (auto* pass : m_passes)
    {
        pass->Execute(ctx);
    }

    EndFrame();
}

void Renderer::UpdateConstantBuffers()
{
    UINT frameIndex = m_swapChain.CurrentBackBufferIndex();
    auto& curFrame = m_frameRes[frameIndex];

    Camera& camera = m_scene.GetCamera();
    DirectionalLight& light = m_scene.GetDirLight();

    // SPCB 갱신 : light shadow
    XMMATRIX lightVP = light.GetLightViewProjMatrix();
    SPConstants SPassData;
    XMStoreFloat4x4(&SPassData.LightViewProj, XMMatrixTranspose(lightVP));

    // FPCB 갱신 : View / Proj
    XMMATRIX view = camera.GetViewMatrix();
    XMMATRIX proj = camera.GetProjMatrix();
    XMMATRIX viewProj = view * proj;

    FPConstants FPassData;
    XMStoreFloat4x4(&FPassData.View, XMMatrixTranspose(view));
    XMStoreFloat4x4(&FPassData.Proj, XMMatrixTranspose(proj));
    XMStoreFloat4x4(&FPassData.ViewProj, XMMatrixTranspose(viewProj));
    FPassData.EyePos = camera.GetPosition();
    FPassData.DirLight = light;
    XMStoreFloat4x4(&FPassData.LightViewProj, XMMatrixTranspose(lightVP));

    curFrame.SPCB->CopyData(0, SPassData);
    curFrame.FPCB->CopyData(0, FPassData);

    // ObjectCB
    for (auto& item : m_scene.GetRenderItems())
    {
        if (item->NumFramesDirty > 0)
        {
            ObjectConstants objData;
            XMStoreFloat4x4(&objData.World, XMMatrixTranspose(XMLoadFloat4x4(&item->World)));

            curFrame.ObjectCB->CopyData(item->ObjCBIndex, objData);
            item->NumFramesDirty--;
        }
    }
}

void Renderer::DrawImGui()
{
    ImGui::Begin("DX12 Renderer");

    // FPS
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Separator();

    // Camera Info
    Camera& cam = m_scene.GetCamera();
    XMFLOAT3 camPos = cam.GetPosition();
    ImGui::Text("Camera: (%.1f, %.1f, %.1f)", camPos.x, camPos.y, camPos.z);
    ImGui::Separator();

    // Light Controls
    DirectionalLight& light = m_scene.GetDirLight();
    ImGui::Text("Directional Light");
    ImGui::DragFloat3("Direction", &light.Direction.x, 0.01f, -1.0f, 1.0f);
    ImGui::ColorEdit3("Color", &light.Color.x);
    ImGui::SliderFloat("Intensity", &light.Intensity, 0.0f, 5.0f);
    ImGui::Separator();

    // Render Stats
    ImGui::Text("Objects: %d", m_scene.GetObjectCount());
    ImGui::Text("Passes: %d", (int)m_passes.size());
    ImGui::Text("Shadow Map: %dx%d", m_shadowPass->GetShadowMap().Size,
        m_shadowPass->GetShadowMap().Size);

    ImGui::End();
}

void Renderer::EndFrame()
{
    auto* cmdList = m_commandList.Get();

    // Barrier: RENDER_TARGET -> PRESENT
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_swapChain.CurrentBackBuffer();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
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

    m_scene.GetCamera().SetAspectRatio((float)width / height);

    for (auto* pass : m_passes)
        pass->OnResize(m_device.GetDevice(), width, height);
}

void Renderer::Shutdown()
{
    m_commandQueue.Shutdown();
}
