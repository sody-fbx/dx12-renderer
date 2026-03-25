#pragma once

// ═══════════════════════════════════════════════════════════════════
//  FrameResource.h - 셰이더에 전달할 CB 구조체
// ═══════════════════════════════════════════════════════════════════

#include "Core/MathHelper.h"
#include "Resource/UploadBuffer.h"
#include "Scene/Light.h"

struct ObjectConstants
{
    XMFLOAT4X4 World = {};

    ObjectConstants()
    {
        XMStoreFloat4x4(&World, XMMatrixIdentity());
    }
};

struct FPConstants          //ForwardPassConstants
{
    XMFLOAT4X4 View       = {};
    XMFLOAT4X4 Proj       = {};
    XMFLOAT4X4 ViewProj   = {};
    XMFLOAT3   EyePos     = { 0.0f, 0.0f, 0.0f };
    float      Padding1    = 0.0f;   // 16바이트 정렬용
    XMFLOAT4   AmbientLight = { 0.1f, 0.1f, 0.1f, 1.0f };

    DirectionalLight DirLight;
    XMFLOAT4X4 LightViewProj = {};
    
    FPConstants()
    {
        XMStoreFloat4x4(&View, XMMatrixIdentity());
        XMStoreFloat4x4(&Proj, XMMatrixIdentity());
        XMStoreFloat4x4(&ViewProj, XMMatrixIdentity());
        XMStoreFloat4x4(&LightViewProj, XMMatrixIdentity());
    }
};

struct SPConstants          // ShadowPassConstants
{
    XMFLOAT4X4 LightViewProj = {};

    SPConstants()
    {
        XMStoreFloat4x4(&LightViewProj, XMMatrixIdentity());
    }
};

struct FrameResource
{
    std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;
    std::unique_ptr<UploadBuffer<FPConstants>>   FPCB   = nullptr;
    std::unique_ptr<UploadBuffer<SPConstants>>   SPCB   = nullptr;

    // 이 프레임의 GPU 작업이 완료되었는지 추적하는 Fence 값
    UINT64 FenceValue = 0;

    void Initialize(ID3D12Device* device, UINT objectCount)
    {
        ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(device, objectCount, true);
        FPCB     = std::make_unique<UploadBuffer<FPConstants>>(device, 1, true);
        SPCB     = std::make_unique<UploadBuffer<SPConstants>>(device, 1, true);
    }
};
