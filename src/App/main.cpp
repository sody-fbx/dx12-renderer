// ═══════════════════════════════════════════════════════════════════
//  main.cpp
// ═══════════════════════════════════════════════════════════════════

#include "App/Window.h"
#include "Core/Renderer.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    const int WIDTH  = 1280;
    const int HEIGHT = 720;

    try
    {
        // 1. Window 생성
        Window window;
        window.Initialize(hInstance, WIDTH, HEIGHT, L"DX12 Renderer");
        
        // 2. Renderer 초기화
        Renderer renderer;
        renderer.Initialize(window.GetHWND(), WIDTH, HEIGHT);

        // Resize Callback
        window.OnResize = [&renderer](int w, int h)
        {
            renderer.OnResize(w, h);
        };

        // 4. Main Loop
        while (window.ProcessMessages())
        {
            renderer.Render();
        }

        // 5. 종료
        renderer.Shutdown();
    }
    catch (const std::exception& e)
    {
        MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    return 0;
}
