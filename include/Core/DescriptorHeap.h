#pragma once

// ═══════════════════════════════════════════════════════════════════
//  DescriptorHeap.h — 범용 Descriptor Heap 관리
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"

class DescriptorHeap
{
public:
    void Initialize( ID3D12Device* device
                   , D3D12_DESCRIPTOR_HEAP_TYPE type
                   , UINT numDescriptors
                   , bool shaderVisible );

    // 다음 빈 슬롯에 Descriptor를 할당
    D3D12_CPU_DESCRIPTOR_HANDLE AllocateHandle();
    UINT AllocateIndex();

    // 특정 인덱스의 CPU/GPU 핸들
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(UINT index) const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(UINT index) const;

    ID3D12DescriptorHeap* Get() const;
    UINT GetDescriptorSize() const;
    UINT GetAllocatedCount() const;

private:
    ComPtr<ID3D12DescriptorHeap> m_heap;
    UINT m_descriptorSize  = 0;
    UINT m_numDescriptors  = 0;
    UINT m_currentIndex    = 0;   // 다음 할당 위치
};
