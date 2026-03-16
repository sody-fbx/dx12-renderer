// ═══════════════════════════════════════════════════════════════════
//  Camera.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Scene/Camera.h"

void Camera::Initialize( float distance, float fovDegrees, float aspectRatio,
                         float nearZ, float farZ)
{
    m_radius = distance;
    m_fovY   = MathHelper::DegreesToRadians(fovDegrees);
    m_aspect = aspectRatio;
    m_nearZ  = nearZ;
    m_farZ   = farZ;

    UpdateMatrices();
}

void Camera::Rotate(float dx, float dy)
{
    // 마우스 이동량을 회전 각도로 변환
    // 감도 조절은 여기서
    m_theta -= dx * 0.005f;
    m_phi   -= dy * 0.005f;

    // 짐벌락 예외처리
    m_phi = std::max(0.1f, std::min(XM_PI - 0.1f, m_phi));

    UpdateMatrices();
}

void Camera::Zoom(float delta)
{
    m_radius -= delta * 0.5f;
    m_radius = std::max(1.0f, std::min(50.0f, m_radius));

    UpdateMatrices();
}

void Camera::SetAspectRatio(float aspect)
{
    m_aspect = aspect;
    UpdateMatrices();
}

void Camera::UpdateMatrices()
{
    // 구면 좌표 -> 직교 좌표
    // x = r * sin(phi) * cos(theta)
    // y = r * cos(phi)
    // z = r * sin(phi) * sin(theta)
    float x = m_radius * sinf(m_phi) * cosf(m_theta);
    float y = m_radius * cosf(m_phi);
    float z = m_radius * sinf(m_phi) * sinf(m_theta);

    m_position = { x + m_target.x, y + m_target.y, z + m_target.z };

    // View 행렬
    XMVECTOR eye    = XMLoadFloat3(&m_position);
    XMVECTOR target = XMLoadFloat3(&m_target);
    XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMStoreFloat4x4(&m_view, XMMatrixLookAtLH(eye, target, up));

    // Projection 행렬
    XMStoreFloat4x4(&m_proj, XMMatrixPerspectiveFovLH(m_fovY, m_aspect, m_nearZ, m_farZ));
}

XMMATRIX Camera::GetViewMatrix() const
{
    return XMLoadFloat4x4(&m_view);
}

XMMATRIX Camera::GetProjMatrix() const
{
    return XMLoadFloat4x4(&m_proj);
}

XMFLOAT3 Camera::GetPosition() const
{
    return m_position;
}
