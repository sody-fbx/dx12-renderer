// ═══════════════════════════════════════════════════════════════════
//  main.cpp
// ═══════════════════════════════════════════════════════════════════

#include "App/Window.h"
#include "Core/D3DDevice.h"
#include "Core/CommandQueue.h"
#include "Core/CommandList.h"
#include "Core/SwapChain.h"
#include "Core/RootSignature.h"
#include "Core/PipelineState.h"

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

        // RootSignature 생성
        RootSignature rootSig;
        rootSig.CreateEmpty(device.GetDevice());

        // PipelineState 객체 생성
        PipelineState pso;
        pso.Create( device.GetDevice()
                  , rootSig.Get()
                  , L"src/Shaders/Triangle.hlsl"
                  , L"src/Shaders/Triangle.hlsl"
                  , swapChain.GetBackBufferFormat()
                  , swapChain.GetDepthFormat() );

        // ── TEST : 삼각형 그리기 ──
        struct Vertex
        {
            DirectX::XMFLOAT3 position;
            DirectX::XMFLOAT4 color;
        };

        Vertex vertices[] =
        {
            { {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },  // 상단 빨강
            { {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },  // 우하 초록
            { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },  // 좌하 파랑
        };
        UINT vbSize = sizeof(vertices);

        // 정점 버퍼 생성
        ComPtr<ID3D12Resource> vertexBuffer;

        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC bufferDesc = {};
        bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferDesc.Width = vbSize;
        bufferDesc.Height = 1;
        bufferDesc.DepthOrArraySize = 1;
        bufferDesc.MipLevels = 1;
        bufferDesc.SampleDesc = { 1, 0 };
        bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        ThrowIfFailed(device.GetDevice()->CreateCommittedResource(
            &heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&vertexBuffer)
        ));

        // 정점 데이터 복사
        void* mappedData = nullptr;
        ThrowIfFailed(vertexBuffer->Map(0, nullptr, &mappedData));
        memcpy(mappedData, vertices, vbSize);
        vertexBuffer->Unmap(0, nullptr);

        // Vertex Buffer View 생성
        D3D12_VERTEX_BUFFER_VIEW vbView = {};
        vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
        vbView.SizeInBytes = vbSize;
        vbView.StrideInBytes = sizeof(Vertex);
        
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

            // 렌더타겟 바인딩
            auto rtv = swapChain.CurrentRTV();
            auto dsv = swapChain.DSV();
            cmdList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

            // Viewport & Scissor
            D3D12_VIEWPORT vp = { 0, 0, (float)WIDTH, (float)HEIGHT, 0, 1 };
            D3D12_RECT sc = { 0, 0, WIDTH, HEIGHT };
            cmdList->RSSetViewports(1, &vp);
            cmdList->RSSetScissorRects(1, &sc);

            // PSO & RootSignature
            cmdList->SetPipelineState(pso.Get());
            cmdList->SetGraphicsRootSignature(rootSig.Get());

            // ── TEST : 삼각형 그리기 ──
            cmdList->IASetVertexBuffers(0, 1, &vbView);
            cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            cmdList->DrawInstanced(3, 1, 0, 0);

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
