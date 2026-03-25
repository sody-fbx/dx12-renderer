#pragma once

// ═══════════════════════════════════════════════════════════════════
//  Window.h — Win32 윈도우 래퍼
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"
#include "imgui.h"

#include <functional>

// ImGui Win32 핸들러 (imgui_impl_win32.h)
// Window.cpp에서 호출하기 위해 extern 선언
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class Window
{
public:
    void Initialize(HINSTANCE hInstance, int width, int height, const std::wstring& title);

    bool ProcessMessages();

    // Getters
    HWND  GetHWND()   const { return m_hwnd; }
    int   GetWidth()  const { return m_width; }
    int   GetHeight() const { return m_height; }

    // 리사이즈 콜백
    std::function<void(int, int)> OnResize;

    // 마우스 콜백 추가
    std::function<void(WPARAM, int, int)> OnMouseDown;
    std::function<void(WPARAM, int, int)> OnMouseUp;
    std::function<void(WPARAM, int, int)> OnMouseMove;
    std::function<void(short)>            OnMouseWheel;

private:
    // Win32 메시지 콜백. static이어야 WNDCLASS에 등록 가능.
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND m_hwnd  = nullptr;
    int  m_width  = 0;
    int  m_height = 0;
};
