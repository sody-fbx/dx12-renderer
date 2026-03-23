#pragma once

// ═══════════════════════════════════════════════════════════════════
//  Camera.h — 오비트 카메라
// ═══════════════════════════════════════════════════════════════════

#include "Core/MathHelper.h"

class Camera
{
public:
    void Initialize(float distance, float fovDegrees, float aspectRatio,
                    float nearZ, float farZ);

    void Rotate(float dx, float dy);
    void Zoom(float delta);

    void SetAspectRatio(float aspect);

    XMMATRIX GetViewMatrix()  const;
    XMMATRIX GetProjMatrix()  const;
    XMFLOAT3 GetPosition()    const;

    void UpdateMatrices();

private:
    float m_theta    = XM_PIDIV4;    // 수평 각도 (방위각)
    float m_phi      = XM_PIDIV4;    // 수직 각도 (앙각) — 0이면 위에서, PI면 아래서
    float m_radius   = 5.0f;         // 타겟까지 거리

    XMFLOAT3 m_target = { 0.0f, 0.0f, 0.0f };

    float m_fovY    = XM_PIDIV4;
    float m_aspect  = 1.0f;
    float m_nearZ   = 0.1f;
    float m_farZ    = 100.0f;

    XMFLOAT4X4 m_view = {};
    XMFLOAT4X4 m_proj = {};
    XMFLOAT3   m_position = {};
};
