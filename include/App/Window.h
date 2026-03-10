#pragma once

// ═══════════════════════════════════════════════════════════════════
//  Window.h — Win32 윈도우 래퍼
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"
#include <functional>

class Window
{
public:
    void Initialize(HINSTANCE hInstance, int width, int height, const std::wstring& title);

    bool ProcessMessages();

    // Getters
    HWND  GetHWND()   const { return m_hwnd; }
    int   GetWidth()  const { return m_width; }
    int   GetHeight() const { return m_height; }

    // 리사이즈 콜백 — SwapChain 재생성 등에 연결
    std::function<void(int, int)> OnResize;

private:
    // Win32 메시지 콜백. static이어야 WNDCLASS에 등록 가능.
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND m_hwnd  = nullptr;
    int  m_width  = 0;
    int  m_height = 0;
};
