#pragma once

// ═══════════════════════════════════════════════════════════════════
//  Light.h
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"

// 셰이더의 MAX_POINT_LIGHTS 정의와 반드시 일치해야 함
static constexpr int MAX_POINT_LIGHTS = 8;
static constexpr int MAX_SPOT_LIGHTS  = 4;

//  PointLight — 위치 기반 구형 감쇠 광원
//  셰이더의 PointLightData 구조체와 레이아웃이 동일해야 함 (32 bytes)
struct PointLight
{
    XMFLOAT3 Position  = { 0.0f, 1.0f, 0.0f };
    float    Radius    = 5.0f;                   // 감쇠 반경
    XMFLOAT3 Color     = { 1.0f, 1.0f, 1.0f };
    float    Intensity = 1.0f;
};

//  SpotLight — 원뿔형 방향성 광원
//  셰이더의 SpotLightData 구조체와 레이아웃이 동일해야 함 (64 bytes)
struct SpotLight
{
    XMFLOAT3 Position      = { 0.0f, 5.0f, 0.0f };
    float    Radius        = 15.0f;

    XMFLOAT3 Direction     = { 0.0f, -1.0f, 0.0f };
    float    InnerCosAngle = 0.9659f;   // cos(15°) — 이 각도 안쪽은 전체 강도

    XMFLOAT3 Color         = { 1.0f, 1.0f, 1.0f };
    float    Intensity     = 2.0f;

    float    OuterCosAngle = 0.8660f;   // cos(30°) — 이 각도 밖은 감쇠 0
    XMFLOAT3 Padding       = {};
};

//  DirectionalLight — 방향성 광원
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

//  Lights — Scene이 보유하는 모든 광원을 하나로 묶은 집합체
//  Renderer는 이 구조체 하나만 참조하면 된다
struct Lights
{
    DirectionalLight        Directional;
    std::vector<PointLight> Points;
    std::vector<SpotLight>  Spots;
};
