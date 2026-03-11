// ═══════════════════════════════════════════════════════════════════
//  main.cpp
// ═══════════════════════════════════════════════════════════════════

#include "App/Window.h"
#include "Core/D3DDevice.h"
#include "Core/CommandQueue.h"
#include "Core/CommandList.h"
#include "Core/SwapChain.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    const int WIDTH  = 1280;
    const int HEIGHT = 720;

    try
    {
        // Window 생성
        Window window;
        window.Initialize(hInstance, WIDTH, HEIGHT, L"DX12 Renderer");

        // D3DDevice 생성 : GPU Connect
        D3DDevice device;
        device.Initialize();

        // CommandQueue 생성
        CommandQueue commandQueue;
        commandQueue.Initialize(device.GetDevice());

        // SwapChain 생성
        SwapChain swapChain;
        swapChain.Initialize( device.GetFactory()
                            , commandQueue.GetQueue()
                            , device.GetDevice()
                            , window.GetHWND()
                            , WIDTH
                            , HEIGHT);

        // CommandList 생성
        CommandList commandList;
        commandList.Initialize(device.GetDevice());

        // Back Buffer 추적 번호
        UINT64 frameFenceValues[FRAME_BUFFER_COUNT] = { };
        while (window.ProcessMessages())
        {
            UINT frameIdx = swapChain.CurrentBackBufferIndex();

            // Fence 대기
            commandQueue.WaitForFenceValue(frameFenceValues[frameIdx]);

            // CommandList 리셋
            commandList.Reset(frameIdx, nullptr);
            auto* cmdList = commandList.Get();

            // Barrier: PRESENT → RENDER_TARGET
            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Transition.pResource = swapChain.CurrentBackBuffer();
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            cmdList->ResourceBarrier(1, &barrier);

            // Clear
            const float color[] = { 0.05f, 0.05f, 0.15f, 1.0f };
            cmdList->ClearRenderTargetView(swapChain.CurrentRTV(), color, 0, nullptr);
            cmdList->ClearDepthStencilView(swapChain.DSV(),
                D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

            // Barrier: RENDER_TARGET → PRESENT
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
            cmdList->ResourceBarrier(1, &barrier);

            // Resent
            commandList.Close();
            commandQueue.ExecuteCommandList(cmdList);
            swapChain.Present(true);
            frameFenceValues[frameIdx] = commandQueue.Signal();
            
        }

        commandQueue.Shutdown();
    }
    catch (const std::exception& e)
    {
        MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    return 0;
}
