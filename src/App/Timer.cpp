// ═══════════════════════════════════════════════════════════════════
//  Timer.cpp
// ═══════════════════════════════════════════════════════════════════

#include "App/Timer.h"

void Timer::Initialize()
{
    QueryPerformanceFrequency(&m_frequency);
    QueryPerformanceCounter(&m_prevTime);
}

void Timer::Tick()
{
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);

    m_deltaTime = static_cast<float>(currentTime.QuadPart - m_prevTime.QuadPart)
                / static_cast<float>(m_frequency.QuadPart);
    m_prevTime  = currentTime;
    m_totalTime += m_deltaTime;

    // FPS 계산 (1초마다 갱신)
    m_frameCount++;
    m_elapsed += m_deltaTime;
    if (m_elapsed >= 1.0f)
    {
        m_fps        = m_frameCount;
        m_frameCount = 0;
        m_elapsed   -= 1.0f;
    }
}
