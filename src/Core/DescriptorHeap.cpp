// ═══════════════════════════════════════════════════════════════════
//  DescriptorHeap.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Core/DescriptorHeap.h"

void DescriptorHeap::Initialize( ID3D12Device* device
                               , D3D12_DESCRIPTOR_HEAP_TYPE type
                               , UINT numDescriptors
                               , bool shaderVisible )
{
    m_numDescriptors = numDescriptors;

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = numDescriptors;
    desc.Type           = type;
    desc.Flags          = shaderVisible
                            ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
                            : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heap)));

    m_descriptorSize = device->GetDescriptorHandleIncrementSize(type);

    m_currentIndex = 0;
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::AllocateHandle()
{
    assert(m_currentIndex < m_numDescriptors && "Descriptor heap full!");

    D3D12_CPU_DESCRIPTOR_HANDLE handle = GetCPUHandle(m_currentIndex);
    m_currentIndex++;
    return handle;
}

UINT DescriptorHeap::AllocateIndex()
{
    assert(m_currentIndex < m_numDescriptors && "Descriptor heap full!");

    D3D12_CPU_DESCRIPTOR_HANDLE handle = GetCPUHandle(m_currentIndex);
    
    return m_currentIndex++;
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUHandle(UINT index) const
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_heap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<SIZE_T>(index) * m_descriptorSize;
    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUHandle(UINT index) const
{
    D3D12_GPU_DESCRIPTOR_HANDLE handle = m_heap->GetGPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<UINT64>(index) * m_descriptorSize;
    return handle;
}

ID3D12DescriptorHeap* DescriptorHeap::Get() const
{ 
    return m_heap.Get(); 
}

UINT DescriptorHeap::GetDescriptorSize() const
{ 
    return m_descriptorSize;
}

UINT DescriptorHeap::GetAllocatedCount() const
{ 
    return m_currentIndex; 
}
