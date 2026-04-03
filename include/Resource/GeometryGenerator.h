#pragma once

// ═══════════════════════════════════════════════════════════════════
//  GeometryGenerator.h — 기본 도형 생성
//  추후 OBJ/glTF 로더로 교체 또는 병행
// ═══════════════════════════════════════════════════════════════════

#include "Resource/Vertex.h"
#include <vector>

namespace GeometryGenerator
{
    template<typename v>
    struct MeshData
    {
        std::vector<v>          Vertices;
        std::vector<uint32_t>   Indices;
    };

    // Triangle
    MeshData<VertexCol> CreateTriangle();

    // Box
    MeshData<VertexTex> CreateBox(float width, float height, float depth);

    // Sphere
    MeshData<VertexTex> CreateSphere(float radius, UINT sliceCount, UINT stackCount);

    // Grid
    MeshData<VertexTex> CreateGrid(float width, float depth, UINT m, UINT n);

    // Cylinder
    MeshData<VertexTex> CreateCylinder( float bottomRadius, float topRadius,
                                        float height, UINT sliceCount, UINT stackCount );

    // 탄젠트 계산
    void ComputeTangents( std::vector<VertexTex>& vertices
                        , const std::vector<uint32_t>& indices );
}
