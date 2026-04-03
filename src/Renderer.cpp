// ═══════════════════════════════════════════════════════════════════
//  Renderer.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Renderer.h"

// 전체 SRV 슬롯 수
// Fallback(1) + 텍스처(최대 64) + Shadow(1) + @
static constexpr UINT SRV_HEAP_SIZE = 128;

void Renderer::Initialize(HWND hwnd, int width, int height)
{
    m_hwnd   = hwnd;
    m_width  = width;
    m_height = height;

    // DX12 Core 초기화
    m_device.Initialize();
    m_commandQueue.Initialize(m_device.GetDevice());
    m_swapChain.Initialize( m_device.GetFactory()
                          , m_commandQueue.GetQueue()
                          , m_device.GetDevice()
                          , hwnd, width, height );
    m_commandList.Initialize(m_device.GetDevice());

    m_srvHeap.Initialize( m_device.GetDevice()
                        , D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
                        , SRV_HEAP_SIZE
                        , true );

    // 사용할 Scene 등록
    m_sceneManager.Register();

    // Scene에 사용되는 리소스 사전 로딩
    m_sceneManager.PreloadResources(m_meshManager, m_texManager);

    // 리소스 GPU 업로드
    m_commandList.Reset(0, nullptr);
    m_meshManager.BuildAll(m_device.GetDevice(), m_commandList.Get());
    m_texManager.BuildAll(m_device.GetDevice(), m_commandList.Get(), m_srvHeap);
    m_commandList.Close();
    m_commandQueue.ExecuteCommandList(m_commandList.Get());
    m_commandQueue.Flush();
    m_meshManager.ReleaseUploadBuffers();
    m_texManager.ReleaseUploadBuffers();

    // RenderContext 구성
    BuildRenderContext();

    // 모든 Scene RenderItem 생성
    m_sceneManager.Generate(m_renderCtx);

    // FrameResource 할당 (전체 Scene 오브젝트 합산 크기)
    const UINT totalObjects = m_sceneManager.GetTotalObjectCount();
    for (auto& fr : m_frameRes)
        fr.Initialize(m_device.GetDevice(), totalObjects);

    // 초기 활성 Scene 설정
    m_sceneManager.SetActiveScene("demo", (float)m_width / m_height);

    BuildPasses();
}

void Renderer::BuildRenderContext()
{
    m_renderCtx.SrvHeap            = &m_srvHeap;
    m_renderCtx.Meshes             = &m_meshManager;
    m_renderCtx.Textures           = &m_texManager;
}

void Renderer::BuildPasses()
{
    m_shadowPass = std::make_unique<ShadowPass>();
    m_shadowPass->Setup(m_device.GetDevice(), m_width, m_height, m_srvHeap);

    // Forward 패스 — ForwardPass 모드에서 사용
    m_forwardPass = std::make_unique<ForwardPass>();
    m_forwardPass->Setup(m_device.GetDevice(), m_width, m_height,
                         &m_shadowPass->GetShadowMap(), &m_renderCtx);

    // Deferred 패스 — Deferred 모드에서 사용
    m_geometryPass = std::make_unique<GeometryPass>();
    m_geometryPass->Setup(m_device.GetDevice(), m_width, m_height, m_srvHeap, &m_renderCtx);

    m_lightingPass = std::make_unique<LightingPass>();
    m_lightingPass->Setup(m_device.GetDevice(),
                          &m_shadowPass->GetShadowMap(),
                          &m_geometryPass->GetGBuffer(),
                          &m_renderCtx);

    m_imGuiPass = std::make_unique<ImGuiPass>();
    m_imGuiPass->Setup(m_device.GetDevice(), m_width, m_height);
    m_imGuiPass->InitBackend(m_device.GetDevice(), m_commandQueue.GetQueue(), m_hwnd);

    // m_passes = 모든 패스 등록 — OnResize 시 한꺼번에 처리
    m_passes.clear();
    m_passes.push_back(m_shadowPass.get());
    m_passes.push_back(m_forwardPass.get());
    m_passes.push_back(m_geometryPass.get());
    m_passes.push_back(m_lightingPass.get());
    m_passes.push_back(m_imGuiPass.get());
}

void Renderer::Update(float mouseDx, float mouseDy, float wheelDelta)
{
    // ImGui가 마우스를 캡처 중이면 Scene 카메라 조작 안 함
    if (ImGui::GetIO().WantCaptureMouse) return;

    Scene* scene = m_sceneManager.GetActiveScene();

    if (mouseDx != 0.0f || mouseDy != 0.0f)
        scene->GetCamera().Rotate(mouseDx, mouseDy);

    if (wheelDelta != 0.0f)
        scene->GetCamera().Zoom(wheelDelta);
}

void Renderer::BeginFrame()
{
    UINT frameIndex = m_swapChain.CurrentBackBufferIndex();

    m_commandQueue.WaitForFenceValue(m_frameFenceValues[frameIndex]);
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
    UpdateConstantBuffers();

    m_imGuiPass->BeginFrame();
    DrawImGui();

    UINT frameIndex = m_swapChain.CurrentBackBufferIndex();

    FrameContext ctx        = {};
    ctx.CmdList             = m_commandList.Get();
    ctx.BackBuffer          = m_swapChain.CurrentBackBuffer();
    ctx.RTV                 = m_swapChain.CurrentRTV();
    ctx.FrameIndex          = frameIndex;
    ctx.ScreenWidth         = m_width;
    ctx.ScreenHeight        = m_height;
    ctx.CurrentFrameResource = &m_frameRes[frameIndex];
    ctx.RenderItems         = &m_sceneManager.GetActiveScene()->GetRenderItems();

    // ShadowPass는 항상 실행
    m_shadowPass->Execute(ctx);

    // 렌더링 모드에 따라 분기
    if (m_useDeferredRendering)
    {
        m_geometryPass->Execute(ctx);
        m_lightingPass->Execute(ctx);
    }
    else
    {
        m_forwardPass->Execute(ctx);
    }

    m_imGuiPass->Execute(ctx);

    EndFrame();
}

void Renderer::UpdateConstantBuffers()
{
    UINT frameIndex = m_swapChain.CurrentBackBufferIndex();
    auto& curFrame  = m_frameRes[frameIndex];

    Scene*             scene  = m_sceneManager.GetActiveScene();
    Camera&            cam    = scene->GetCamera();
    const Lights&      lights = scene->GetLights();

    // ShadowCB
    XMMATRIX lightVP = lights.Directional.GetLightViewProjMatrix();
    ShadowPassConstants shadowData;
    XMStoreFloat4x4(&shadowData.LightViewProj, XMMatrixTranspose(lightVP));
    curFrame.ShadowCB->CopyData(0, shadowData);

    // PassCB
    XMMATRIX view     = cam.GetViewMatrix();
    XMMATRIX proj     = cam.GetProjMatrix();
    XMMATRIX viewProj = view * proj;

    PassConstants passData;
    XMStoreFloat4x4(&passData.View,          XMMatrixTranspose(view));
    XMStoreFloat4x4(&passData.Proj,          XMMatrixTranspose(proj));
    XMStoreFloat4x4(&passData.ViewProj,      XMMatrixTranspose(viewProj));
    XMStoreFloat4x4(&passData.LightViewProj, XMMatrixTranspose(lightVP));
    passData.EyePos   = cam.GetPosition();
    passData.DirLight = lights.Directional;

    // PointLights
    passData.PointLightCount = (int)lights.Points.size();
    for (int i = 0; i < passData.PointLightCount; i++)
        passData.PointLights[i] = lights.Points[i];

    // SpotLights
    passData.SpotLightCount = (int)lights.Spots.size();
    for (int i = 0; i < passData.SpotLightCount; i++)
        passData.SpotLights[i] = lights.Spots[i];

    curFrame.PassCB->CopyData(0, passData);

    // ObjectCB — 활성 Scene의 아이템만 갱신
    for (auto& item : scene->GetRenderItems())
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

    // 렌더링 모드 선택
    ImGui::Text("Rendering Mode");
    if (ImGui::RadioButton("Forward",  !m_useDeferredRendering))
        m_useDeferredRendering = false;
    ImGui::SameLine();
    if (ImGui::RadioButton("Deferred",  m_useDeferredRendering))
        m_useDeferredRendering = true;
    ImGui::Separator();

    Scene* scene = m_sceneManager.GetActiveScene();

    // Camera Info
    XMFLOAT3 camPos = scene->GetCamera().GetPosition();
    ImGui::Text("Camera: (%.1f, %.1f, %.1f)", camPos.x, camPos.y, camPos.z);
    ImGui::Separator();

    // Directional Light
    Lights& lights = scene->GetLights();
    ImGui::Text("Directional Light");
    ImGui::DragFloat3("Direction",     &lights.Directional.Direction.x, 0.01f, -1.0f, 1.0f);
    ImGui::ColorEdit3("Dir Color",     &lights.Directional.Color.x);
    ImGui::SliderFloat("Dir Intensity", &lights.Directional.Intensity, 0.0f, 5.0f);
    ImGui::Separator();

    // Point Lights
    ImGui::Text("Point Lights  (%d / %d)", (int)lights.Points.size(), MAX_POINT_LIGHTS);
    if ((int)lights.Points.size() < MAX_POINT_LIGHTS)
        if (ImGui::Button("+ Add Point Light"))
            lights.Points.push_back(PointLight{});

    for (int i = 0; i < (int)lights.Points.size(); i++)
    {
        ImGui::PushID(i);
        char header[32];
        sprintf_s(header, "Point Light [%d]", i);

        if (ImGui::CollapsingHeader(header))
        {
            PointLight& pl = lights.Points[i];
            ImGui::DragFloat3("Position",      &pl.Position.x, 0.1f, -50.0f, 50.0f);
            ImGui::DragFloat ("Radius",        &pl.Radius,     0.1f,   0.1f, 50.0f);
            ImGui::ColorEdit3("PL Color",      &pl.Color.x);
            ImGui::SliderFloat("PL Intensity", &pl.Intensity,  0.0f, 10.0f);

            if (ImGui::Button("Remove"))
            {
                lights.Points.erase(lights.Points.begin() + i);
                ImGui::PopID();
                break;
            }
        }
        ImGui::PopID();
    }
    ImGui::Separator();

    // Spot Lights
    ImGui::Text("Spot Lights  (%d / %d)", (int)lights.Spots.size(), MAX_SPOT_LIGHTS);
    if ((int)lights.Spots.size() < MAX_SPOT_LIGHTS)
        if (ImGui::Button("+ Add Spot Light"))
            lights.Spots.push_back(SpotLight{});

    for (int i = 0; i < (int)lights.Spots.size(); i++)
    {
        ImGui::PushID(1000 + i);
        char header[32];
        sprintf_s(header, "Spot Light [%d]", i);

        if (ImGui::CollapsingHeader(header))
        {
            SpotLight& sl = lights.Spots[i];
            ImGui::DragFloat3("Position",  &sl.Position.x,  0.1f, -50.0f, 50.0f);
            ImGui::DragFloat3("Direction", &sl.Direction.x, 0.01f, -1.0f, 1.0f);
            ImGui::DragFloat ("Radius",    &sl.Radius,      0.1f,   0.1f, 100.0f);

            float innerDeg = acosf(sl.InnerCosAngle) * (180.0f / XM_PI);
            float outerDeg = acosf(sl.OuterCosAngle) * (180.0f / XM_PI);
            if (ImGui::SliderFloat("Inner Angle", &innerDeg, 1.0f, 89.0f))
                sl.InnerCosAngle = cosf(innerDeg * XM_PI / 180.0f);
            if (ImGui::SliderFloat("Outer Angle", &outerDeg, innerDeg + 0.5f, 90.0f))
                sl.OuterCosAngle = cosf(outerDeg * XM_PI / 180.0f);

            ImGui::ColorEdit3 ("SL Color",     &sl.Color.x);
            ImGui::SliderFloat("SL Intensity", &sl.Intensity, 0.0f, 10.0f);

            if (ImGui::Button("Remove"))
            {
                lights.Spots.erase(lights.Spots.begin() + i);
                ImGui::PopID();
                break;
            }
        }
        ImGui::PopID();
    }
    ImGui::Separator();

    // Render Stats
    ImGui::Text("Objects: %d", scene->GetObjectCount());
    ImGui::Text("Passes : %d", m_useDeferredRendering ? 4 : 3);
    ImGui::Separator();

    // 활성 패스의 디버그 UI만 표시
    m_shadowPass->OnDrawDebugUI();
    if (m_useDeferredRendering)
    {
        m_geometryPass->OnDrawDebugUI();
        m_lightingPass->OnDrawDebugUI();
    }
    else
    {
        m_forwardPass->OnDrawDebugUI();
    }

    ImGui::End();
}

void Renderer::EndFrame()
{
    auto* cmdList = m_commandList.Get();

    // Barrier: RENDER_TARGET -> PRESENT
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource   = m_swapChain.CurrentBackBuffer();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    cmdList->ResourceBarrier(1, &barrier);

    m_commandList.Close();
    m_commandQueue.ExecuteCommandList(m_commandList.Get());

	// Fence Signal
    UINT frameIndex = m_swapChain.CurrentBackBufferIndex();
	
	// Present
    m_swapChain.Present(true);
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

    // 카메라 종횡비 갱신
    m_sceneManager.GetActiveScene()->GetCamera().SetAspectRatio((float)width / height);

    for (auto* pass : m_passes)
        pass->OnResize(m_device.GetDevice(), width, height);
}

void Renderer::Shutdown()
{
    m_commandQueue.Shutdown();
}
