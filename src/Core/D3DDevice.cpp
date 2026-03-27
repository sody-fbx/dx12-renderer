// ═══════════════════════════════════════════════════════════════════
//  D3DDevice.cpp — DX12 디바이스 생성
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DDevice.h"

void D3DDevice::Initialize()
{
    // 1. Debug Layer 활성화
    EnableDebugLayer();
    // 2. DXGI Factory 생성
    CreateFactory();
    // 3. 하드웨어 어댑터 선택
    SelectAdapter();
    // 4. ID3D12Device 생성
    CreateDevice();
}

ID3D12Device* D3DDevice::GetDevice()  const
{
    return m_device.Get();
}

IDXGIFactory4* D3DDevice::GetFactory() const
{
    return m_factory.Get();
}

void D3DDevice::EnableDebugLayer()
{
#if defined(_DEBUG)
    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
        debugController->EnableDebugLayer();

        // GPU-based validation
        // 필요 시 아래 주석 해제:
        // ComPtr<ID3D12Debug1> debug1;
        // debugController.As(&debug1);
        // debug1->SetEnableGPUBasedValidation(TRUE);
    }
#endif
}

void D3DDevice::CreateFactory()
{
    // DXGI Factory : GPU(어댑터)를 열거하고, SwapChain을 생성
    UINT factoryFlags = 0;
#if defined(_DEBUG)
    factoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    ThrowIfFailed(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_factory)));
}

void D3DDevice::SelectAdapter()
{
    // 시스템에 GPU가 여러 개일 경우, VRAM이 가장 큰 하드웨어 어댑터를 선택.
    SIZE_T maxVRAM = 0;

    for (UINT i = 0; m_factory->EnumAdapters1(i, &m_adapter) != DXGI_ERROR_NOT_FOUND; ++i)
    {
        DXGI_ADAPTER_DESC1 desc;
        m_adapter->GetDesc1(&desc);

        // 소프트웨어 어댑터 건너뛰기
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            continue;

        // DX12 지원 여부 확인 (Device 생성 시도로 검증)
        if (SUCCEEDED(D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_0,
                                        _uuidof(ID3D12Device), nullptr)))
        {
            if (desc.DedicatedVideoMemory > maxVRAM)
            {
                maxVRAM = desc.DedicatedVideoMemory;
                // 이 어댑터를 선택 (더 좋은 게 나오면 교체)
            }
        }
    }

    // 어댑터를 못 찾으면 WARP 폴백
    if (maxVRAM == 0)
    {
        ThrowIfFailed(m_factory->EnumWarpAdapter(IID_PPV_ARGS(&m_adapter)));
    }
}

void D3DDevice::CreateDevice()
{
    // Feature Level 12.0: DX12의 기본 기능 세트.
    ThrowIfFailed(
        D3D12CreateDevice( m_adapter.Get()
                         , D3D_FEATURE_LEVEL_12_0
                         , IID_PPV_ARGS(&m_device)
        )
    );

    // 심각한 에러에서 브레이크포인트
#if defined(_DEBUG)
    ComPtr<ID3D12InfoQueue> infoQueue;
    if (SUCCEEDED(m_device.As(&infoQueue)))
    {
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
    }
#endif
}
