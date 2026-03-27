#pragma once

// ═══════════════════════════════════════════════════════════════════
//  SceneManager.h — Scene 생명주기 관리
// ═══════════════════════════════════════════════════════════════════

#include "Scene/Scene.h"
#include "Resource/MeshManager.h"
#include "Resource/TextureManager.h"

class SceneManager
{
public:
    // Scene 등록
    // T는 Scene 구체 클래스
    template<typename T>
    void Register(const std::string& name)
    {
        static_assert(std::is_base_of<Scene, T>::value, "T must derive from Scene");
        m_entries.push_back({ name, std::make_unique<T>(), {}, 0 });
    }

    // 사용할 Scene 등록
    void Register();

    // 리소스 사전 로딩
    // 모든 Scene의 MeshManager / TextureManager에 Request.
    void PreloadResources(MeshManager& meshMgr, TextureManager& texMgr);

    // 모든 Scene의 RenderItem 생성
    void Generate(const RenderContext& ctx);

    // 활성 Scene 설정
    void SetActiveScene(const std::string& name, float aspectRatio);

    // Getter
public:
    Scene* GetActiveScene() const;

    // FrameResource ObjectCB 크기
    UINT GetTotalObjectCount() const;

private:
    struct Entry
    {
        std::string            name;
        std::unique_ptr<Scene> scene;
        SceneDesc              desc;        // PreloadResources 시 캐시
        UINT                   cbOffset = 0;
    };

    std::vector<Entry> m_entries;
    Scene*             m_activeScene      = nullptr;
    UINT               m_totalObjectCount = 0;
};
