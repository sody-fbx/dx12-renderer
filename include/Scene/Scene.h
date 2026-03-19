#pragma once

// ═══════════════════════════════════════════════════════════════════
//  Scene.h
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"
#include "Scene/Camera.h"
#include "Scene/RenderItem.h"

#include "Resource/GeometryGenerator.h"

class Scene
{
public:
    void Initialize (ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);

    void SetCamera(Camera cam) { m_camera = cam; }

public: // Getter
    // Camera
    Camera& GetCamera() { return m_camera; }

    // RenderItems
    const std::vector<std::unique_ptr<RenderItem>>& GetRenderItems() const { return m_renderItems; }
    UINT GetObjectCount() const { return (UINT)m_renderItems.size(); }

private:
    void BuildGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
    void BuildRenderItems();

    // Meshes
    std::unordered_map<std::string, std::unique_ptr<Mesh>> m_meshes;

    // RenderItems
    std::vector<std::unique_ptr<RenderItem>> m_renderItems;

    Camera m_camera;
};