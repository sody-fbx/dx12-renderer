// ═══════════════════════════════════════════════════════════════════
//  InputManager.cpp
// ═══════════════════════════════════════════════════════════════════

#include "App/InputManager.h"

void InputManager::OnMouseDown(WPARAM btnState, int x, int y)
{
    if (btnState & MK_LBUTTON)
    {
        m_leftDown = true;
        m_lastX = x;
        m_lastY = y;
    }
}

void InputManager::OnMouseUp(WPARAM btnState, int x, int y)
{
    m_leftDown = false;
}

void InputManager::OnMouseMove(WPARAM btnState, int x, int y)
{
    if (m_leftDown)
    {
        m_deltaX += static_cast<float>(x - m_lastX);
        m_deltaY += static_cast<float>(y - m_lastY);
    }
    m_lastX = x;
    m_lastY = y;
}

void InputManager::OnMouseWheel(short delta)
{
    m_wheelDelta += static_cast<float>(delta) / WHEEL_DELTA;
}

void InputManager::ResetFrameInput()
{
    m_deltaX     = 0.0f;
    m_deltaY     = 0.0f;
    m_wheelDelta = 0.0f;
}
