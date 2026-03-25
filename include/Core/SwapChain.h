#pragma once

// ═══════════════════════════════════════════════════════════════════
//  SwapChain.h — 스왑 체인 & Back Buffer 관리
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"
#include "Core/DescriptorHeap.h"

class SwapChain
{
public:
    void Initialize( IDXGIFactory4* factory
                   , ID3D12CommandQueue* queue
                   , ID3D12Device* device
                   , HWND hwnd
                   , int width
                   , int height );

    void Present(bool vsync);
    void Resize(ID3D12Device* device, int width, int height);

    UINT CurrentBackBufferIndex() const;
    ID3D12Resource* CurrentBackBuffer() const;

    D3D12_CPU_DESCRIPTOR_HANDLE CurrentRTV() const;

private:
    void CreateRTV(ID3D12Device* device);

    ComPtr<IDXGISwapChain4> m_swapChain;

    // Back Buffer
    std::array<ComPtr<ID3D12Resource>, FRAME_BUFFER_COUNT> m_backBuffers;

    // RTV
    DescriptorHeap m_rtvHeap;
};
