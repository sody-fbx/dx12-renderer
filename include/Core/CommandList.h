#pragma once

// ═══════════════════════════════════════════════════════════════════
//  CommandList.h — GPU 명령 기록기
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"

class CommandList
{
public:
    void Initialize(ID3D12Device* device);

    void Reset(UINT frameIndex, ID3D12PipelineState* initialPSO = nullptr);

    void Close();

public: // Getter
    ID3D12GraphicsCommandList* Get() const;

    // 프레임별 Allocator 접근
    ID3D12CommandAllocator* GetAllocator(UINT frameIndex) const;

private:
    std::array<ComPtr<ID3D12CommandAllocator>, FRAME_BUFFER_COUNT> m_allocators;
    ComPtr<ID3D12GraphicsCommandList> m_cmdList;
};
