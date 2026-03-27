// ═══════════════════════════════════════════════════════════════════
//  SceneManager.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Scene/SceneManager.h"

#include "Scene/DemoScene.h"

void SceneManager::Register()
{
    Register<DemoScene>("demo");
}

void SceneManager::PreloadResources(MeshManager& meshMgr, TextureManager& texMgr)
{
    for (auto& entry : m_entries)
    {
        // Scene별 SceneDesc 캐시
        entry.desc = entry.scene->CreateSceneDesc();

        for (const auto& mesh : entry.desc.Meshes)
        {
            if (mesh.SourceType == MeshSourceType::Primitive)
                meshMgr.RequestPrimitive(mesh.Name, mesh.Primitive, mesh.Params);
            else
                meshMgr.RequestFile(mesh.Name, mesh.FilePath);
        }

        for (const auto& tex : entry.desc.Textures)
            texMgr.Request(tex.Name, tex.Path);
    }
}

void SceneManager::Generate(const RenderContext& ctx)
{
    UINT cbOffset = 0;

    for (auto& entry : m_entries)
    {
        entry.cbOffset = cbOffset;
        entry.scene->Generate(entry.desc, ctx, cbOffset);

        // 다음 Scene의 CB 시작 슬롯 = 현재 Scene 오브젝트 수만큼 전진
        cbOffset += static_cast<UINT>(entry.desc.Items.size());
    }

    m_totalObjectCount = cbOffset;
}

void SceneManager::SetActiveScene(const std::string& name, float aspectRatio)
{
    for (auto& entry : m_entries)
    {
        if (entry.name == name)
        {
            m_activeScene = entry.scene.get();
            m_activeScene->OnActivate(aspectRatio);
            return;
        }
    }

    OutputDebugStringA("[SceneManager] Scene not found: ");
    OutputDebugStringA(name.c_str());
    OutputDebugStringA("\n");
}

Scene* SceneManager::GetActiveScene() const
{
    return m_activeScene;
}

UINT SceneManager::GetTotalObjectCount() const
{
    if (m_totalObjectCount == 0)
    {
        OutputDebugStringA("[SceneManager] Scene object not generate : Requires call Generate()");
    }

    return m_totalObjectCount;
}
