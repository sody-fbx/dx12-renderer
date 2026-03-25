// ═══════════════════════════════════════════════════════════════════
//  SwapChain.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Core/SwapChain.h"

void SwapChain::Initialize( IDXGIFactory4* factory
                          , ID3D12CommandQueue* queue
                          , ID3D12Device* device
                          , HWND hwnd
                          , int width, int height)
{
    // SwapChain 생성
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    desc.Width       = width;
    desc.Height      = height;
    desc.Format      = BACK_BUFFER_FORMAT;
    desc.SampleDesc  = { 1, 0 };            // MSAA 미사용
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = FRAME_BUFFER_COUNT;
    desc.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.Flags       = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    ComPtr<IDXGISwapChain1> swapChain1;
    ThrowIfFailed( factory->CreateSwapChainForHwnd( queue
                                                  , hwnd
                                                  , &desc
                                                  , nullptr
                                                  , nullptr
                                                  , &swapChain1 ));

    ThrowIfFailed(swapChain1.As(&m_swapChain));

    // RTV 생성
    CreateRTV(device);
}

void SwapChain::CreateRTV(ID3D12Device* device)
{
    m_rtvHeap.Initialize( device
                        , D3D12_DESCRIPTOR_HEAP_TYPE_RTV
                        , FRAME_BUFFER_COUNT
                        , false);

    for (UINT i = 0; i < FRAME_BUFFER_COUNT; ++i)
    {
        ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i])));
        device->CreateRenderTargetView( m_backBuffers[i].Get()
                                      , nullptr
                                      , m_rtvHeap.Allocate());
    }
}

void SwapChain::Present(bool vsync)
{
    // Present
    // vsync=true -> SyncInterval=1 (모니터 주사율에 맞춤)
    // vsync=false -> SyncInterval=0 + TEARING (무제한 FPS)
    UINT syncInterval = vsync ? 1 : 0;
    UINT flags        = vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING;
    ThrowIfFailed(m_swapChain->Present(syncInterval, flags));
}

void SwapChain::Resize(ID3D12Device* device, int width, int height)
{
    // 리사이즈 처리
    // 주의: 호출 전에 Flush 필요
    // 기존 Back Buffer 참조 해제
    for (auto& buffer : m_backBuffers)
        buffer.Reset();

    // SwapChain 버퍼 리사이즈
    ThrowIfFailed(
        m_swapChain->ResizeBuffers(
            FRAME_BUFFER_COUNT,
            width, height,
            BACK_BUFFER_FORMAT,
            DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING
        )
    );

    // RTV 재생성
    CreateRTV(device);
}

UINT SwapChain::CurrentBackBufferIndex() const
{
    return m_swapChain->GetCurrentBackBufferIndex();
}

ID3D12Resource* SwapChain::CurrentBackBuffer() const
{
    return m_backBuffers[CurrentBackBufferIndex()].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE SwapChain::CurrentRTV() const
{
    return m_rtvHeap.GetCPUHandle(CurrentBackBufferIndex());
}
