#pragma once

// ═══════════════════════════════════════════════════════════════════
//  D3DDevice.h — DX12 디바이스 관리
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"

class D3DDevice
{
public:
    void Initialize();

public: // Getter
    ID3D12Device*   GetDevice()  const { return m_device.Get(); }
    IDXGIFactory4*  GetFactory() const { return m_factory.Get(); }

private:
    void EnableDebugLayer();
    void CreateFactory();
    void SelectAdapter();
    void CreateDevice();

    ComPtr<IDXGIFactory4>  m_factory;
    ComPtr<IDXGIAdapter1>  m_adapter;
    ComPtr<ID3D12Device>   m_device;
};
