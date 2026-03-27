#pragma once

// ═══════════════════════════════════════════════════════════════════
//  InputManager.h — 마우스/키보드 입력 관리
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"

class InputManager
{
public:
    // WndProc에서 호출
    void OnMouseDown(WPARAM btnState, int x, int y);
    void OnMouseUp(WPARAM btnState, int x, int y);
    void OnMouseMove(WPARAM btnState, int x, int y);
    void OnMouseWheel(short delta);

    float GetDeltaX()       const;
    float GetDeltaY()       const;
    float GetWheelDelta()   const;
    bool  IsLeftButtonDown() const;

    // 델타값 리셋
    void ResetFrameInput();

private:
    int   m_lastX      = 0;
    int   m_lastY      = 0;
    float m_deltaX     = 0.0f;
    float m_deltaY     = 0.0f;
    float m_wheelDelta = 0.0f;
    bool  m_leftDown   = false;
};
