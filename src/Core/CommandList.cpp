// ═══════════════════════════════════════════════════════════════════
//  CommandList.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Core/CommandList.h"

void CommandList::Initialize(ID3D12Device* device)
{
    // CommandAllocator 생성
    for (UINT i = 0; i < FRAME_BUFFER_COUNT; ++i)
    {
        ThrowIfFailed(
            device->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(&m_allocators[i])
            )
        );
    }

    // GraphicsCommandList 생성
    // Allocator[0]으로 초기 생성. 매 프레임 Reset으로 Allocator를 교체.
    ThrowIfFailed(
        device->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            m_allocators[0].Get(),
            nullptr,               // Reset 시 지정되기에 초기는 null
            IID_PPV_ARGS(&m_cmdList)
        )
    );

    // 생성 직후 Open 상태이므로 Close해둠.
    // 메인 루프에서 Reset으로 다시 Open할 것.
    m_cmdList->Close();
}

void CommandList::Reset(UINT frameIndex, ID3D12PipelineState* initialPSO)
{
    // 프레임 시작 시 리셋
    ThrowIfFailed(m_allocators[frameIndex]->Reset());
    ThrowIfFailed(m_cmdList->Reset(m_allocators[frameIndex].Get(), initialPSO));
}

void CommandList::Close()
{
    // 명령 기록 완료. ExecuteCommandLists 호출 전에 반드시 Close.
    ThrowIfFailed(m_cmdList->Close());
}
