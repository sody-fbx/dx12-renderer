#pragma once

// ═══════════════════════════════════════════════════════════════════
//  RenderItem.h
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"
#include "Core/MathHelper.h"
#include "Resource/Mesh.h"

struct RenderItem
{
    XMFLOAT4X4 World;

    // 해당 오브젝트가 참조하는 Mesh
    Mesh* MeshRef = nullptr;

    // CB에서 해당 오브젝트의 인덱스
    UINT ObjCBIndex = 0;

    // CB 갱신 필요 수
    int NumFramesDirty = FRAME_BUFFER_COUNT;

    RenderItem()
    {
        XMStoreFloat4x4(&World, XMMatrixIdentity());
    }
};
