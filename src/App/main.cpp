// ═══════════════════════════════════════════════════════════════════
//  main.cpp
// ═══════════════════════════════════════════════════════════════════

#include "App/Window.h"
#include "Core/D3DDevice.h"

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

        while (window.ProcessMessages())
        {
            
        }
    }
    catch (const std::exception& e)
    {
        MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    return 0;
}
