#pragma once

// ═══════════════════════════════════════════════════════════════════
//  UploadBuffer.h
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"

template<typename T>
class UploadBuffer
{
public:
    UploadBuffer( ID3D12Device* device
                , UINT elementCount
                , bool isConstantBuffer )
                : m_isConstantBuffer(isConstantBuffer)
    {
        m_elementByteSize = sizeof(T);

        // CB는 256바이트 정렬 필수
        if (isConstantBuffer)
            m_elementByteSize = (sizeof(T) + 255) & ~255;

        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC bufferDesc = {};
        bufferDesc.Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferDesc.Width            = static_cast<UINT64>(m_elementByteSize) * elementCount;
        bufferDesc.Height           = 1;
        bufferDesc.DepthOrArraySize = 1;
        bufferDesc.MipLevels        = 1;
        bufferDesc.SampleDesc       = { 1, 0 };
        bufferDesc.Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        ThrowIfFailed(device->CreateCommittedResource( &heapProps, D3D12_HEAP_FLAG_NONE
                                                     , &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ
                                                     , nullptr, IID_PPV_ARGS(&m_buffer) ));

        // 생성 시 Map, 소멸 시 Unmap — 수명 내내 매핑 유지
        ThrowIfFailed(m_buffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedData)));
    }

    ~UploadBuffer()
    {
        if (m_buffer)
            m_buffer->Unmap(0, nullptr);
        m_mappedData = nullptr;
    }

    // 복사/이동 금지
    UploadBuffer(const UploadBuffer&) = delete;
    UploadBuffer& operator=(const UploadBuffer&) = delete;

    // 특정 인덱스의 데이터를 갱신
    // 매 프레임 MVP 행렬 등을 업데이트할 때 사용
    void CopyData(UINT elementIndex, const T& data)
    {
        memcpy(m_mappedData + static_cast<UINT64>(elementIndex) * m_elementByteSize,
               &data, sizeof(T));
    }

    ID3D12Resource* GetResource() const { return m_buffer.Get(); }

    // GPU 가상 주소 (Root Descriptor로 바인딩 시 사용)
    D3D12_GPU_VIRTUAL_ADDRESS GetElementGPUAddress(UINT elementIndex) const
    {
        return m_buffer->GetGPUVirtualAddress()
             + static_cast<UINT64>(elementIndex) * m_elementByteSize;
    }

    UINT GetElementByteSize() const { return m_elementByteSize; }

private:
    ComPtr<ID3D12Resource> m_buffer;
    BYTE*                  m_mappedData     = nullptr;
    UINT                   m_elementByteSize = 0;
    bool                   m_isConstantBuffer = false;
};
