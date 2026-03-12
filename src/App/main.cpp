// ═══════════════════════════════════════════════════════════════════
//  main.cpp
// ═══════════════════════════════════════════════════════════════════

#include "App/Window.h"
#include "Core/Renderer.h"
#include "App/Timer.h"

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

        // 3. 타이머 초기화
        Timer timer;
        timer.Initialize();

        // 4. Main Loop
        while (window.ProcessMessages())
        {
            timer.Tick();
            renderer.Render();

            // 타이틀바에 FPS 표시
            if (timer.FPS() > 0)
            {
                std::wstring title = L"DX12 Renderer — FPS: "
                                   + std::to_wstring(timer.FPS());
                SetWindowText(window.GetHWND(), title.c_str());
            }
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
