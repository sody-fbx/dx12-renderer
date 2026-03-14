#pragma once

// ═══════════════════════════════════════════════════════════════════
//  Mesh.h — Vertex & Index Buffer 관리
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"

struct SubMesh
{
    UINT IndexCount         = 0;
    UINT StartIndexLocation = 0;
    INT  BaseVertexLocation = 0;
};

class Mesh
{
public:
    void Create( ID3D12Device* device
               , ID3D12GraphicsCommandList* cmdList
               , const void* vertexData, UINT vertexCount, UINT vertexStride
               , const void* indexData,  UINT indexCount,  DXGI_FORMAT indexFormat);

    D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const;
    D3D12_INDEX_BUFFER_VIEW  IndexBufferView()  const;

    UINT GetIndexCount() const { return m_indexCount; }

    // SubMesh 관리
    std::unordered_map<std::string, SubMesh> SubMeshes;

    std::string Name;

private:
    ComPtr<ID3D12Resource> m_vertexBufferGPU;
    ComPtr<ID3D12Resource> m_indexBufferGPU;

    ComPtr<ID3D12Resource> m_vertexBufferUpload;
    ComPtr<ID3D12Resource> m_indexBufferUpload;

    UINT        m_vertexCount      = 0;
    UINT        m_vertexStride     = 0;
    UINT        m_indexCount       = 0;
    DXGI_FORMAT m_indexFormat      = DXGI_FORMAT_R32_UINT;
};
