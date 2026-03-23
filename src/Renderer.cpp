// ═══════════════════════════════════════════════════════════════════
//  Renderer.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Renderer.h"

#include "Scene/Camera.h"

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

    m_passes.clear();
    m_passes.push_back(m_shadowPass.get());
    m_passes.push_back(m_forwardPass.get());
}

void Renderer::Update(float mouseDx, float mouseDy, float wheelDelta)
{
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

void Renderer::EndFrame()
{
    auto* cmdList = m_commandList.Get();

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
