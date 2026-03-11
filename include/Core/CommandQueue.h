#pragma once

// ═══════════════════════════════════════════════════════════════════
//  CommandQueue.h — GPU 작업 제출 & Fence 동기화
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"

class CommandQueue
{
public:
    void Initialize(ID3D12Device* device);
    void Shutdown();

    // CommandList를 GPU에 제출
    void ExecuteCommandList(ID3D12CommandList* cmdList);

    // 현재 프레임의 Fence 값을 Signal하고 반환
    UINT64 Signal();

    // 특정 Fence 값까지 GPU 완료를 대기
    void WaitForFenceValue(UINT64 fenceValue);

    // GPU의 모든 작업이 끝날 때까지 대기 (Flush)
    void Flush();

    ID3D12CommandQueue* GetQueue() const { return m_queue.Get(); }

private:
    ComPtr<ID3D12CommandQueue> m_queue;
    ComPtr<ID3D12Fence>        m_fence;
    HANDLE                     m_fenceEvent = nullptr;
    UINT64                     m_fenceValue = 0;
};
