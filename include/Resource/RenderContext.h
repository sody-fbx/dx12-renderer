#pragma once

// ═══════════════════════════════════════════════════════════════════
//  RenderContext.h
//  Scene과 Render에서 설정 후 ForwardPass로 전달
// ═══════════════════════════════════════════════════════════════════

#include "Core/DescriptorHeap.h"
#include "Resource/MeshManager.h"
#include "Resource/TextureManager.h"

struct RenderContext
{
    DescriptorHeap* SrvHeap            = nullptr;
    MeshManager*    Meshes             = nullptr;
    TextureManager* Textures           = nullptr;
};