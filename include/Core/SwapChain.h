#pragma once

// ═══════════════════════════════════════════════════════════════════
//  SwapChain.h — 스왑 체인 & Back Buffer 관리
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"

class SwapChain
{
public:
    void Initialize(IDXGIFactory4* factory, ID3D12CommandQueue* queue,
                    ID3D12Device* device, HWND hwnd, int width, int height);

    void Present(bool vsync);
    void Resize(ID3D12Device* device, int width, int height);

    UINT CurrentBackBufferIndex() const;

    ID3D12Resource* CurrentBackBuffer() const;

    D3D12_CPU_DESCRIPTOR_HANDLE CurrentRTV() const;

    D3D12_CPU_DESCRIPTOR_HANDLE DSV() const;

    DXGI_FORMAT GetBackBufferFormat() const { return m_backBufferFormat; }
    DXGI_FORMAT GetDepthFormat()      const { return m_depthFormat; }

private:
    void CreateRTVs(ID3D12Device* device);
    void CreateDepthBuffer(ID3D12Device* device, int width, int height);

    ComPtr<IDXGISwapChain4> m_swapChain;

    // Back Buffer 리소스 & RTV Heap
    std::array<ComPtr<ID3D12Resource>, FRAME_BUFFER_COUNT> m_backBuffers;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    UINT m_rtvDescriptorSize = 0;

    // Depth/Stencil Buffer & DSV Heap
    ComPtr<ID3D12Resource>       m_depthBuffer;
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

    DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT m_depthFormat      = DXGI_FORMAT_D24_UNORM_S8_UINT;
};
