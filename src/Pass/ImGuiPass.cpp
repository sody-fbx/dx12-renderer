// ═══════════════════════════════════════════════════════════════════
//  ImGuiPass.cpp — ImGui DX12 통합
//  ImGui GitHub: https://github.com/ocornut/imgui
// ═══════════════════════════════════════════════════════════════════

#include "Pass/ImGuiPass.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

// SRV 콜백에서 접근하기 위한 static 포인터
static DescriptorHeap* g_imguiSrvHeap = nullptr;

void ImGuiPass::Setup(ID3D12Device* device, int width, int height)
{
    m_srvHeap.Initialize( device
                        , D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
                        , 1       // Font 텍스처 1개
                        , true ); // Shader Visible

    // ImGui 컨텍스트 생성
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // 스타일 설정
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding   = 6.0f;
    style.FrameRounding    = 4.0f;
    style.GrabRounding     = 4.0f;
    style.ScrollbarRounding = 4.0f;
}

void ImGuiPass::InitBackend( ID3D12Device* device
                           , ID3D12CommandQueue* commandQueue
                           , HWND hwnd)
{
    // Win32 백엔드
    ImGui_ImplWin32_Init(hwnd);

    // DX12 백엔드
    g_imguiSrvHeap = &m_srvHeap;

    ImGui_ImplDX12_InitInfo initInfo = {};
    initInfo.Device = device;
    initInfo.CommandQueue = commandQueue;
    initInfo.NumFramesInFlight = FRAME_BUFFER_COUNT;
    initInfo.RTVFormat = BACK_BUFFER_FORMAT;
    initInfo.SrvDescriptorHeap = m_srvHeap.Get();

    initInfo.SrvDescriptorAllocFn = []( ImGui_ImplDX12_InitInfo*
                                      , D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu
                                      , D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu)
        {
            *out_cpu = g_imguiSrvHeap->Allocate();
            *out_gpu = g_imguiSrvHeap->GetGPUHandle(0);
        };

    initInfo.SrvDescriptorFreeFn = []( ImGui_ImplDX12_InitInfo*
                                     , D3D12_CPU_DESCRIPTOR_HANDLE
                                     , D3D12_GPU_DESCRIPTOR_HANDLE )
        {
            // Font SRV는 앱 수명 동안 유지 — 해제 불필요
        };

    ImGui_ImplDX12_Init(&initInfo);
    m_initialized = true;
}

void ImGuiPass::BeginFrame()
{
    if (!m_initialized) return;

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiPass::Execute(const FrameContext& ctx)
{
    if (!m_initialized) return;

    // ImGui 드로우 데이터 확정
    ImGui::Render();

    auto* cmdList = ctx.CmdList;

    // ImGui의 Font SRV Heap 바인딩
    ID3D12DescriptorHeap* heaps[] = { m_srvHeap.Get() };
    cmdList->SetDescriptorHeaps(1, heaps);

    // ImGui 드로우 명령을 CommandList에 기록
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList);
}

void ImGuiPass::OnResize(ID3D12Device* device, int width, int height)
{
    // ImGui는 자체적으로 DisplaySize를 갱신하므로 별도 처리 불필요
}

void ImGuiPass::Shutdown()
{
    if (!m_initialized) return;

    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    m_initialized = false;
}
