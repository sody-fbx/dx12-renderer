#pragma once

// ═══════════════════════════════════════════════════════════════════
//  Vertex.h — 정점 포맷 정의
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"

struct VertexCol
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT4 Color;
};

struct VertexTex
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT2 TexCoord;
    DirectX::XMFLOAT3 Tangent  = { 1.0f, 0.0f, 0.0f };  // UV U축 방향 (접선 벡터)
};
