#pragma once

// ═══════════════════════════════════════════════════════════════════
//  MathHelper.h — DirectXMath Rapper & Util
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"

using namespace DirectX;

namespace MathHelper
{
    // 항등 행렬
    inline XMFLOAT4X4 Identity4x4()
    {
        XMFLOAT4X4 I;
        XMStoreFloat4x4(&I, XMMatrixIdentity());
        return I;
    }

    // 도(degree) → 라디안 변환
    inline float DegreesToRadians(float degrees)
    {
        return degrees * (XM_PI / 180.0f);
    }

    // float3 생성 헬퍼
    inline XMFLOAT3 Float3(float x, float y, float z)
    {
        return XMFLOAT3(x, y, z);
    }
}
