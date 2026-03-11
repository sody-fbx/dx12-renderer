// ═══════════════════════════════════════════════════════════════════
//  SwapChain.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Core/SwapChain.h"

void SwapChain::Initialize(IDXGIFactory4* factory, ID3D12CommandQueue* queue,
                            ID3D12Device* device, HWND hwnd, int width, int height)
{
    // SwapChain 생성
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    desc.Width       = width;
    desc.Height      = height;
    desc.Format      = m_backBufferFormat;
    desc.SampleDesc  = { 1, 0 };            // MSAA 미사용
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = FRAME_BUFFER_COUNT;
    desc.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.Flags       = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    ComPtr<IDXGISwapChain1> swapChain1;
    ThrowIfFailed(
        factory->CreateSwapChainForHwnd(
            queue,
            hwnd,
            &desc,
            nullptr,    // Fullscreen desc
            nullptr,    // Restrict to output
            &swapChain1
        )
    );

    ThrowIfFailed(swapChain1.As(&m_swapChain));

    // RTV & Depth Buffer 생성
    CreateRTVs(device);
    CreateDepthBuffer(device, width, height);
}

void SwapChain::CreateRTVs(ID3D12Device* device)
{
    // RTV (Render Target View) Descriptor Heap
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FRAME_BUFFER_COUNT;
    rtvHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

    m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV
    );

    // 각 Back Buffer에 대해 RTV 생성
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

    for (UINT i = 0; i < FRAME_BUFFER_COUNT; ++i)
    {
        ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i])));
        device->CreateRenderTargetView(m_backBuffers[i].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += m_rtvDescriptorSize;
    }
}

void SwapChain::CreateDepthBuffer(ID3D12Device* device, int width, int height)
{
    // Depth/Stencil Buffer
    D3D12_RESOURCE_DESC depthDesc = {};
    depthDesc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Width              = width;
    depthDesc.Height             = height;
    depthDesc.DepthOrArraySize   = 1;
    depthDesc.MipLevels          = 1;
    depthDesc.Format             = m_depthFormat;
    depthDesc.SampleDesc         = { 1, 0 };
    depthDesc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthDesc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format               = m_depthFormat;
    clearValue.DepthStencil.Depth   = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;   // GPU 전용 메모리

    ThrowIfFailed(
        device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &depthDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,   // 초기 상태: Depth 쓰기
            &clearValue,
            IID_PPV_ARGS(&m_depthBuffer)
        )
    );

    // DSV (Depth Stencil View) Heap
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    ThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

    device->CreateDepthStencilView(
        m_depthBuffer.Get(), nullptr,
        m_dsvHeap->GetCPUDescriptorHandleForHeapStart()
    );
}

void SwapChain::Present(bool vsync)
{
    // Present
    // vsync=true → SyncInterval=1 (모니터 주사율에 맞춤)
    // vsync=false → SyncInterval=0 + TEARING (무제한 FPS)
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
    m_depthBuffer.Reset();

    // SwapChain 버퍼 리사이즈
    ThrowIfFailed(
        m_swapChain->ResizeBuffers(
            FRAME_BUFFER_COUNT,
            width, height,
            m_backBufferFormat,
            DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING
        )
    );

    // RTV & Depth Buffer 재생성
    CreateRTVs(device);
    CreateDepthBuffer(device, width, height);
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
    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<SIZE_T>(CurrentBackBufferIndex()) * m_rtvDescriptorSize;
    return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE SwapChain::DSV() const
{
    return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}
