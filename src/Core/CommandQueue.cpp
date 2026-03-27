// ═══════════════════════════════════════════════════════════════════
//  CommandQueue.cpp — GPU 작업 제출 & Fence 동기화
// ═══════════════════════════════════════════════════════════════════

#include "Core/CommandQueue.h"

void CommandQueue::Initialize(ID3D12Device* device)
{
    // Command Queue 생성
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type     = D3D12_COMMAND_LIST_TYPE_DIRECT;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_queue)));

    // Fence 생성
    ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

    // Fence Event
    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!m_fenceEvent)
        throw std::runtime_error("Failed to create fence event");
}

void CommandQueue::Shutdown()
{
    // 종료 전 GPU 작업 모두 완료 대기
    Flush();

    if (m_fenceEvent)
    {
        CloseHandle(m_fenceEvent);
        m_fenceEvent = nullptr;
    }
}

void CommandQueue::ExecuteCommandList(ID3D12CommandList* cmdList)
{
    // CommandList 제출
    ID3D12CommandList* lists[] = { cmdList };
    m_queue->ExecuteCommandLists(1, lists);
}

UINT64 CommandQueue::Signal()
{
    // Fence Signal
    m_fenceValue++;
    ThrowIfFailed(m_queue->Signal(m_fence.Get(), m_fenceValue));
    return m_fenceValue;
}

void CommandQueue::WaitForFenceValue(UINT64 fenceValue)
{
    // GPU 완료 작업 수 < CPU Fence 값 (새로 그릴 Back Buffer가 다 차있음)
    if (m_fence->GetCompletedValue() < fenceValue)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent));

        // CPU 스레드 블로킹
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}

void CommandQueue::Flush()
{
    // GPU 전체 플러시
    UINT64 fenceValue = Signal();
    WaitForFenceValue(fenceValue);
}

ID3D12CommandQueue* CommandQueue::GetQueue() const
{ 
    return m_queue.Get(); 
}
