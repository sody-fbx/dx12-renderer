#pragma once

// ═══════════════════════════════════════════════════════════════════
//  Light.h — 라이트 데이터 구조체
// ═══════════════════════════════════════════════════════════════════

#include "Core/MathHelper.h"

struct DirectionalLight
{
    // 셰이더에 전달될 데이터 (16바이트 정렬 주의)
    XMFLOAT3 Direction = { 0.5f, -1.0f, 0.3f };
    float    Padding1  = 0.0f;
    XMFLOAT3 Color     = { 1.0f, 1.0f, 0.9f };
    float    Intensity = 1.0f;
};
