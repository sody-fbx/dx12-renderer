// ═══════════════════════════════════════════════════════════════════
//  main.cpp
// ═══════════════════════════════════════════════════════════════════

#include "App/Window.h"
#include "App/Timer.h"
#include "App/InputManager.h"

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

        // 3. Application 초기화
        Timer timer;
        timer.Initialize();

        InputManager inputManager;
        window.OnMouseDown = [&](WPARAM btn, int x, int y) 
        {
            inputManager.OnMouseDown(btn, x, y);
        };
        window.OnMouseUp = [&](WPARAM btn, int x, int y)
        {
            inputManager.OnMouseUp(btn, x, y);
        };
        window.OnMouseMove = [&](WPARAM btn, int x, int y)
        {
            inputManager.OnMouseMove(btn, x, y);
        };
        window.OnMouseWheel = [&](short delta)
        {
            inputManager.OnMouseWheel(delta);
        };

        // 4. Main Loop
        while (window.ProcessMessages())
        {
            timer.Tick();
            renderer.Update( inputManager.GetDeltaX()
                           , inputManager.GetDeltaY()
                           , inputManager.GetWheelDelta() );
            renderer.Render();
            inputManager.ResetFrameInput();

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
