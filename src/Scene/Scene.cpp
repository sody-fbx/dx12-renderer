// ═══════════════════════════════════════════════════════════════════
//  Scene.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Scene/Scene.h"

void Scene::Generate(const SceneDesc& desc, const RenderContext& ctx, UINT cbOffset)
{
    m_renderItems.clear();
    BuildRenderItems(desc.Items, ctx, cbOffset);
}

void Scene::BuildRenderItems( const std::vector<RenderItemDesc>& items
                            , const RenderContext& ctx
                            , UINT cbOffset )
{
    UINT cbIndex = cbOffset;

    for (const auto& desc : items)
        AddRenderItem(ctx, desc.MeshName, desc.TextureName, desc.NormalMapName, XMLoadFloat4x4(&desc.World), cbIndex);
}

void Scene::AddRenderItem( const RenderContext& rctx
                         , const std::string& meshName
                         , const std::string& texName
                         , const std::string& normalMapName
                         , XMMATRIX world
                         , UINT& cbIndex )
{
    Mesh* mesh = rctx.Meshes->Get(meshName);
    Texture* tex = rctx.Textures->Get(texName);

    if (mesh == nullptr)
    {
        OutputDebugStringA("[Scene] Mesh not found, skipping RenderItem: ");
        OutputDebugStringA(meshName.c_str());
        OutputDebugStringA("\n");
        return;
    }

    // 노말맵: 이름이 있으면 룩업, 없으면 nullptr
    Texture* normalMap = normalMapName.empty() ? nullptr : rctx.Textures->Get(normalMapName);

    auto item           = std::make_unique<RenderItem>();
    item->MeshRef       = mesh;
    item->TexRef        = tex;
    item->NormalMapRef  = normalMap;
    item->ObjCBIndex    = cbIndex++;
    XMStoreFloat4x4(&item->World, world);
    m_renderItems.push_back(std::move(item));
}

void Scene::SetCamera(Camera cam)
{
    m_camera = cam;
}

Camera& Scene::GetCamera()
{
    return m_camera;
}

DirectionalLight& Scene::GetDirLight()
{ 
    return m_lights.Directional; 
}

Lights& Scene::GetLights() 
{ 
    return m_lights; 
}

const Lights& Scene::GetLights() const
{ 
    return m_lights; 
}

const std::vector<std::unique_ptr<RenderItem>>& Scene::GetRenderItems() const
{
    return m_renderItems;
}

UINT Scene::GetObjectCount() const
{
    return static_cast<UINT>(m_renderItems.size());
}