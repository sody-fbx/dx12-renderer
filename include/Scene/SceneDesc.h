#pragma once

// ═══════════════════════════════════════════════════════════════════
//  SceneDesc.h — Scene 설정 데이터 구조
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"
#include "Resource/PrimitiveDesc.h"

// 메쉬 소스 타입
enum class MeshSourceType { Primitive, File };

// 메쉬 에셋 정의
struct MeshDesc
{
    std::string     Name;
    MeshSourceType  SourceType = MeshSourceType::Primitive;
    MeshPrimitive   Primitive  = MeshPrimitive::Box;
    PrimitiveParams Params     = {};
    std::wstring    FilePath;

    // 절차적 메쉬용
    static MeshDesc FromPrimitive(const std::string& name, MeshPrimitive prim, PrimitiveParams params = {})
    {
        MeshDesc d;
        d.Name       = name;
        d.SourceType = MeshSourceType::Primitive;
        d.Primitive  = prim;
        d.Params     = params;
        return d;
    }

    // 파일 로드용 (.obj)
    static MeshDesc FromFile(const std::string& name, const std::wstring& path)
    {
        MeshDesc d;
        d.Name       = name;
        d.SourceType = MeshSourceType::File;
        d.FilePath   = path;
        return d;
    }
};

// 텍스처 에셋 정의
struct TextureDesc
{
    std::string     Name;
    std::wstring    Path;
};

// Scene 오브젝트 정의
struct RenderItemDesc
{
    std::string MeshName;
    std::string TextureName;
    XMFLOAT4X4  World;

    RenderItemDesc( const std::string& mesh
                  , const std::string& tex
                  , XMMATRIX world = XMMatrixIdentity())
                  : MeshName(mesh), TextureName(tex)
    {
        XMStoreFloat4x4(&World, world);
    }
};

// Scene 전체 설정
struct SceneDesc
{
    std::vector<MeshDesc>       Meshes;
    std::vector<TextureDesc>    Textures;
    std::vector<RenderItemDesc> Items;
};
