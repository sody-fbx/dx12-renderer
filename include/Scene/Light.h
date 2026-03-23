#pragma once

// ═══════════════════════════════════════════════════════════════════
//  Light.h
// ═══════════════════════════════════════════════════════════════════

#include "Core/MathHelper.h"

struct DirectionalLight
{
    XMFLOAT3 Direction = { 0.5f, -1.0f, 0.3f };
    float    Padding1  = 0.0f;
    XMFLOAT3 Color     = { 1.0f, 1.0f, 0.9f };
    float    Intensity = 1.0f;

    XMMATRIX GetLightViewMatrix() const
    {
        XMVECTOR lightDir = XMVector3Normalize(XMLoadFloat3(&Direction));
        XMVECTOR lightPos = XMVectorScale(lightDir, -20.0f);
        XMVECTOR target   = XMVectorZero();
        XMVECTOR up       = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        return XMMatrixLookAtLH(lightPos, target, up);
    }

    XMMATRIX GetLightProjMatrix() const
    {
        return XMMatrixOrthographicLH(20.0f, 20.0f, 0.1f, 50.0f);
    }

    XMMATRIX GetLightViewProjMatrix() const
    {
        return GetLightViewMatrix() * GetLightProjMatrix();
    }
};
