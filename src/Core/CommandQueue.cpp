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
    // 여러 CommandList를 한 번에 제출할 수도 있음.
    // 멀티스레드로 CommandList를 병렬 기록할 때 유용 (추후 확장 가능).
    ID3D12CommandList* lists[] = { cmdList };
    m_queue->ExecuteCommandLists(1, lists);
}

UINT64 CommandQueue::Signal()
{
    // Fence Signal
    // "지금까지 제출된 모든 명령이 끝나면 Fence를 이 값으로 설정해줘"
    // GPU 쪽에서 비동기적으로 처리됨.
    m_fenceValue++;
    ThrowIfFailed(m_queue->Signal(m_fence.Get(), m_fenceValue));
    return m_fenceValue;
}

void CommandQueue::WaitForFenceValue(UINT64 fenceValue)
{
    // Fence 대기
    // GPU가 아직 해당 값에 도달하지 않았으면 CPU가 대기.
    // 이미 완료된 경우 즉시 통과.
    if (m_fence->GetCompletedValue() < fenceValue)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent));

        // CPU 스레드 블로킹 — GPU가 끝날 때까지 여기서 멈춤.
        // 타임아웃을 INFINITE 대신 적절한 값으로 설정하면
        // GPU 행(hang)을 감지할 수 있음.
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}

void CommandQueue::Flush()
{
    // GPU 전체 플러시
    // 현재까지 제출된 모든 작업이 완료될 때까지 대기.
    // 리사이즈, 종료 시 등 리소스를 안전하게 해제해야 할 때 사용.
    UINT64 fenceValue = Signal();
    WaitForFenceValue(fenceValue);
}
