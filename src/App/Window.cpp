// ═══════════════════════════════════════════════════════════════════
//  Window.cpp — Win32 윈도우 구현
// ═══════════════════════════════════════════════════════════════════

#include "App/Window.h"

// WndProc에서 Window 인스턴스에 접근하기 위한 글로벌 포인터.
static Window* g_window = nullptr;

void Window::Initialize(HINSTANCE hInstance, int width, int height,
                         const std::wstring& title)
{
    g_window = this;
    m_width  = width;
    m_height = height;

    // 윈도우 클래스 등록
    WNDCLASSEX wc = {};
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"DX12RendererClass";
    RegisterClassEx(&wc);

    // 클라이언트 영역 크기 보정
    // width/height는 "그리는 영역"의 크기.
    // 타이틀바, 테두리 포함한 전체 윈도우 크기로 보정.
    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    // 윈도우 생성
    m_hwnd = CreateWindowEx(
        0,
        L"DX12RendererClass",
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!m_hwnd)
        throw std::runtime_error("Failed to create window");

    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
}

bool Window::ProcessMessages()
{
    // 메시지 펌프. false 반환 = WM_QUIT 수신 -> 앱 종료.
    MSG msg = {};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
            return false;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // ImGui가 입력을 처리하면 true 반환
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_window && g_window->OnResize)
        {
            int w = LOWORD(lParam);
            int h = HIWORD(lParam);
            if (w > 0 && h > 0)
            {
                g_window->m_width  = w;
                g_window->m_height = h;
                g_window->OnResize(w, h);
            }
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
            PostQuitMessage(0);
        return 0;

    case WM_LBUTTONDOWN:
        if (g_window && g_window->OnMouseDown)
            g_window->OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        SetCapture(hwnd);
        return 0;

    case WM_LBUTTONUP:
        if (g_window && g_window->OnMouseUp)
            g_window->OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        ReleaseCapture();
        return 0;

    case WM_MOUSEMOVE:
        if (g_window && g_window->OnMouseMove)
            g_window->OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;

    case WM_MOUSEWHEEL:
        if (g_window && g_window->OnMouseWheel)
            g_window->OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
