// ═══════════════════════════════════════════════════════════════════
//  Scene.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Scene/Scene.h"

void Scene::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
    BuildGeometry(device, cmdList);
    BuildRenderItems();
}

void Scene::BuildGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
    // Box
    {
        auto data = GeometryGenerator::CreateBox(1.0f, 1.0f, 1.0f);
        auto mesh = std::make_unique<Mesh>();
        mesh->Create( device, cmdList,
                      data.Vertices.data(), (UINT)data.Vertices.size(), sizeof(VertexTex),
                      data.Indices.data(), (UINT)data.Indices.size(), DXGI_FORMAT_R32_UINT
        );
        mesh->Name = "Box";
        m_meshes["Box"] = std::move(mesh);
    }

    // Sphere
    {
        auto data = GeometryGenerator::CreateSphere(0.5f, 20, 20);
        auto mesh = std::make_unique<Mesh>();
        mesh->Create( device, cmdList,
                      data.Vertices.data(), (UINT)data.Vertices.size(), sizeof(VertexTex),
                      data.Indices.data(), (UINT)data.Indices.size(), DXGI_FORMAT_R32_UINT
        );
        mesh->Name = "Sphere";
        m_meshes["Sphere"] = std::move(mesh);
    }

    // Cylinder
    {
        auto data = GeometryGenerator::CreateCylinder(0.4f, 0.4f, 1.5f, 20, 5);
        auto mesh = std::make_unique<Mesh>();
        mesh->Create( device, cmdList,
                      data.Vertices.data(), (UINT)data.Vertices.size(), sizeof(VertexTex),
                      data.Indices.data(), (UINT)data.Indices.size(), DXGI_FORMAT_R32_UINT
        );
        mesh->Name = "Cylinder";
        m_meshes["Cylinder"] = std::move(mesh);
    }

    // Grid
    {
        auto data = GeometryGenerator::CreateGrid(10.0f, 10.0f, 20, 20);
        auto mesh = std::make_unique<Mesh>();
        mesh->Create( device, cmdList,
                      data.Vertices.data(), (UINT)data.Vertices.size(), sizeof(VertexTex),
                      data.Indices.data(), (UINT)data.Indices.size(), DXGI_FORMAT_R32_UINT
        );
        mesh->Name = "Grid";
        m_meshes["Grid"] = std::move(mesh);
    }
}

void Scene::BuildRenderItems()
{
    UINT cbIndex = 0;

    // Grid (바닥, y=0)
    {
        auto item = std::make_unique<RenderItem>();
        item->MeshRef = m_meshes["Grid"].get();
        item->ObjCBIndex = cbIndex++;
        // World = Identity (원점에 수평으로 깔림)
        m_renderItems.push_back(std::move(item));
    }

    // Box (원점 위, y=0.5)
    {
        auto item = std::make_unique<RenderItem>();
        item->MeshRef = m_meshes["Box"].get();
        item->ObjCBIndex = cbIndex++;
        XMStoreFloat4x4(&item->World, XMMatrixTranslation(0.0f, 0.5f, 0.0f));
        m_renderItems.push_back(std::move(item));
    }

    // Sphere (오른쪽, x=3)
    {
        auto item = std::make_unique<RenderItem>();
        item->MeshRef = m_meshes["Sphere"].get();
        item->ObjCBIndex = cbIndex++;
        XMStoreFloat4x4(&item->World, XMMatrixTranslation(3.0f, 0.5f, 0.0f));
        m_renderItems.push_back(std::move(item));
    }

    // Cylinder (왼쪽, x=-3)
    {
        auto item = std::make_unique<RenderItem>();
        item->MeshRef = m_meshes["Cylinder"].get();
        item->ObjCBIndex = cbIndex++;
        XMStoreFloat4x4(&item->World, XMMatrixTranslation(-3.0f, 0.75f, 0.0f));
        m_renderItems.push_back(std::move(item));
    }
}