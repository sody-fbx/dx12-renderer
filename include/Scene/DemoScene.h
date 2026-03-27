#pragma once

// ═══════════════════════════════════════════════════════════════════
//  DemoScene.h — 기본 데모 Scene
// ═══════════════════════════════════════════════════════════════════

#include "Scene/Scene.h"

class DemoScene : public Scene
{
public:
    SceneDesc CreateSceneDesc() const override;
    void      OnActivate(float aspectRatio) override;
};
