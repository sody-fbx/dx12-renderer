#pragma once

// ═══════════════════════════════════════════════════════════════════
//  MeshManager.h — Mesh 리소스 관리
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"

#include "Resource/Mesh.h"
#include "Resource/GeometryGenerator.h"
#include "Resource/PrimitiveDesc.h"

class MeshManager
{
public:
    void RequestPrimitive( const std::string& name
                         , MeshPrimitive type
                         , PrimitiveParams params = {} );
    void RequestFile(const std::string& name, const std::wstring& path);

    void BuildAll(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);

    void ReleaseUploadBuffers();

    bool IsReady() const;

public: // Getter
    Mesh* Get(const std::string& name) const;

private:
    struct PrimitiveRequest
    {
        std::string     name;
        MeshPrimitive   type;
        PrimitiveParams params;
    };

    struct FileRequest
    {
        std::string     name;
        std::wstring    path;
    };

    void BuildPrimitive( ID3D12Device* device
                       , ID3D12GraphicsCommandList* cmdList
                       , const PrimitiveRequest& req );

    void BuildFile( ID3D12Device* device
                  , ID3D12GraphicsCommandList* cmdList
                  , const FileRequest& req );

    std::vector<PrimitiveRequest> m_primitiveRequests;
    std::vector<FileRequest>      m_fileRequests;

    std::unordered_map<std::string, std::unique_ptr<Mesh>> m_meshes;

    bool m_built = false;
};
