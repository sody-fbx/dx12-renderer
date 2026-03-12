#pragma once

// ═══════════════════════════════════════════════════════════════════
//  Timer.h - Delta time & FPS 측정
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"

class Timer
{
public:
    void  Initialize();
    void  Tick();              // 매 프레임 호출

    float DeltaTime()   const { return m_deltaTime; }
    float TotalTime()   const { return m_totalTime; }
    int   FPS()         const { return m_fps; }

private:
    LARGE_INTEGER m_frequency   = {};
    LARGE_INTEGER m_prevTime    = {};
    float         m_deltaTime   = 0.0f;
    float         m_totalTime   = 0.0f;

    int           m_frameCount  = 0;
    float         m_elapsed     = 0.0f;
    int           m_fps         = 0;
};
