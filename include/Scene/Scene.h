#pragma once

// ═══════════════════════════════════════════════════════════════════
//  Scene.h — 추상 베이스 클래스
// ═══════════════════════════════════════════════════════════════════

#include "Core/D3DUtil.h"

#include "Scene/Camera.h"
#include "Scene/RenderItem.h"
#include "Scene/Light.h"
#include "Scene/SceneDesc.h"

#include "Resource/RenderContext.h"

class Scene
{
public:
    virtual ~Scene() = default;

    // 각 Scene이 필요한 에셋 목록 반환
    virtual SceneDesc CreateSceneDesc() const = 0;

    // SceneDesc 기반 RenderItem 생성
    // cbOffset: SceneManager가 배분한 ObjectCB 시작 슬롯
    void Generate(const SceneDesc& desc, const RenderContext& ctx, UINT cbOffset = 0);

    // 해당 Scene 활성화
    virtual void OnActivate(float aspectRatio) {}

    void SetCamera(Camera cam);

public: // Getter
    Camera&             GetCamera();
    DirectionalLight&   GetDirLight();

    Lights&             GetLights();
    const Lights&       GetLights() const;

    const std::vector<std::unique_ptr<RenderItem>>& GetRenderItems() const;
    UINT GetObjectCount() const;

protected:
    Camera      m_camera;
    Lights      m_lights;

private:
    void BuildRenderItems( const std::vector<RenderItemDesc>& items
                         , const RenderContext& ctx
                         , UINT cbOffset );
    void AddRenderItem( const RenderContext& ctx
                      , const std::string& meshName
                      , const std::string& texName
                      , XMMATRIX world
                      , UINT& cbIndex );

    std::vector<std::unique_ptr<RenderItem>> m_renderItems;
};