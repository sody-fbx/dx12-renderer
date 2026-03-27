// ═══════════════════════════════════════════════════════════════════
//  Mesh.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Resource/Mesh.h"

// Upload Heap -> Default Heap 복사를 위한 헬퍼 함수
static ComPtr<ID3D12Resource> CreateDefaultBuffer( ID3D12Device* device
                                                 , ID3D12GraphicsCommandList* cmdList
                                                 , const void* data
                                                 , UINT64 byteSize
                                                 , ComPtr<ID3D12Resource>& uploadBuffer )
{
    ComPtr<ID3D12Resource> defaultBuffer;

    // 1. Default Heap 최종 버퍼 생성
    D3D12_HEAP_PROPERTIES defaultHeap = {};
    defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width            = byteSize;
    bufferDesc.Height           = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels        = 1;
    bufferDesc.SampleDesc       = { 1, 0 };
    bufferDesc.Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    ThrowIfFailed(device->CreateCommittedResource( &defaultHeap, D3D12_HEAP_FLAG_NONE
                                                 , &bufferDesc, D3D12_RESOURCE_STATE_COMMON
                                                 , nullptr, IID_PPV_ARGS(&defaultBuffer) ));

    // 2. Upload Heap 임시 버퍼 생성
    D3D12_HEAP_PROPERTIES uploadHeap = {};
    uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;

    ThrowIfFailed(device->CreateCommittedResource( &uploadHeap, D3D12_HEAP_FLAG_NONE
                                                 , &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ
                                                 , nullptr, IID_PPV_ARGS(&uploadBuffer) ));

    // 3. CPU에서 Upload Heap에 데이터 복사
    void* mappedData = nullptr;
    ThrowIfFailed(uploadBuffer->Map(0, nullptr, &mappedData));
    memcpy(mappedData, data, byteSize);
    uploadBuffer->Unmap(0, nullptr);

    // 4. GPU에게 Upload -> Default 복사 명령 기록
    // Barrier: COMMON -> COPY_DEST
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource   = defaultBuffer.Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    cmdList->ResourceBarrier(1, &barrier);

    // 복사 명령
    cmdList->CopyBufferRegion( defaultBuffer.Get(), 0, uploadBuffer.Get(), 0, byteSize);

    // Barrier: COPY_DEST -> GENERIC_READ
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_GENERIC_READ;
    cmdList->ResourceBarrier(1, &barrier);

    return defaultBuffer;
}

void Mesh::Create( ID3D12Device* device
                 , ID3D12GraphicsCommandList* cmdList
                 , const void* vertexData, UINT vertexCount, UINT vertexStride
                 , const void* indexData,  UINT indexCount,  DXGI_FORMAT indexFormat)
{
    m_vertexCount  = vertexCount;
    m_vertexStride = vertexStride;
    m_indexCount   = indexCount;
    m_indexFormat  = indexFormat;

    UINT vbByteSize = vertexCount * vertexStride;
    UINT ibByteSize = indexCount * (indexFormat == DXGI_FORMAT_R16_UINT ? 2 : 4);

    // Upload -> Default 복사 (GPU 명령으로 기록)
    m_vertexBufferGPU = CreateDefaultBuffer( device
                                           , cmdList
                                           , vertexData
                                           , vbByteSize
                                           , m_vertexBufferUpload);

    m_indexBufferGPU = CreateDefaultBuffer( device
                                          , cmdList
                                          , indexData
                                          , ibByteSize
                                          , m_indexBufferUpload);
}

void Mesh::ReleaseUploadBuffer()
{
    m_vertexBufferUpload.Reset();
    m_indexBufferUpload.Reset();
}

D3D12_VERTEX_BUFFER_VIEW Mesh::VertexBufferView() const
{
    D3D12_VERTEX_BUFFER_VIEW view = {};
    view.BufferLocation = m_vertexBufferGPU->GetGPUVirtualAddress();
    view.SizeInBytes    = m_vertexCount * m_vertexStride;
    view.StrideInBytes  = m_vertexStride;
    return view;
}

D3D12_INDEX_BUFFER_VIEW Mesh::IndexBufferView() const
{
    D3D12_INDEX_BUFFER_VIEW view = {};
    view.BufferLocation = m_indexBufferGPU->GetGPUVirtualAddress();
    view.SizeInBytes    = m_indexCount * (m_indexFormat == DXGI_FORMAT_R16_UINT ? 2 : 4);
    view.Format         = m_indexFormat;
    return view;
}

UINT Mesh::GetIndexCount() const
{
    return m_indexCount;
}

const std::string& Mesh::GetName() const
{
    return m_name;
}

void Mesh::SetName(const std::string& name)
{
    m_name = name;
}
