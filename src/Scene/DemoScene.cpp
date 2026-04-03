// ═══════════════════════════════════════════════════════════════════
//  DemoScene.cpp
//    Meshes   — 사용할 메쉬와 생성 파라미터 (절차적 or 파일)
//    Textures — 사용할 텍스처와 파일 경로
//    Items    — 배치할 오브젝트 (메쉬 이름, 텍스처 이름, 월드 행렬)
// ═══════════════════════════════════════════════════════════════════

#include "Scene/DemoScene.h"

SceneDesc DemoScene::CreateSceneDesc() const
{
    SceneDesc desc;

    desc.Meshes =
    {
        MeshDesc::FromPrimitive("Box",      MeshPrimitive::Box,      PrimitiveParams::Box(1.0f, 1.0f, 1.0f)            ),
        MeshDesc::FromPrimitive("Sphere",   MeshPrimitive::Sphere,   PrimitiveParams::Sphere(0.5f, 20, 20)              ),
        MeshDesc::FromPrimitive("Cylinder", MeshPrimitive::Cylinder, PrimitiveParams::Cylinder(0.4f, 0.4f, 1.5f, 20, 5)),
        MeshDesc::FromPrimitive("Grid",     MeshPrimitive::Grid,     PrimitiveParams::Grid(10.0f, 10.0f, 20, 20)       ),

        // OBJ 파일 추가 시
        // MeshDesc::FromFile("Character", L"assets/character.obj"),
    };

    desc.Textures =
    {
        // Albedo
        { "brick", L"assets/brick.png" },
        { "stone", L"assets/stone.png" },
        { "tile",  L"assets/tile.png"  },
        { "wood",  L"assets/wood.png"  },

        // Normal Maps
        { "brick_normal", L"assets/brick_normal.png" },
        { "stone_normal", L"assets/stone_normal.png" },
        { "tile_normal",  L"assets/tile_normal.png"  },
    };

    // RenderItemDesc(mesh, albedo, world, normalMap="")
    desc.Items =
    {
        { "Grid",     "tile",  XMMatrixIdentity(),                        "tile_normal"  },
        { "Box",      "brick", XMMatrixTranslation( 0.0f, 0.5f,  0.0f),  "brick_normal" },
        { "Sphere",   "stone", XMMatrixTranslation( 3.0f, 0.5f,  0.0f),  "stone_normal" },
        { "Cylinder", "wood",  XMMatrixTranslation(-3.0f, 0.75f, 0.0f)                  },
    };

    return desc;
}

void DemoScene::OnActivate(float aspectRatio)
{
    Camera cam;
    cam.Initialize(5.0f, 45.0f, aspectRatio, 0.1f, 100.0f);
    SetCamera(cam);

    m_lights.Directional.Direction = { 0.5f, -1.0f, 0.3f };
    m_lights.Directional.Color     = { 1.0f, 1.0f, 0.9f  };
    m_lights.Directional.Intensity = 1.0f;

    m_lights.Points.clear();
    m_lights.Spots.clear();
}
