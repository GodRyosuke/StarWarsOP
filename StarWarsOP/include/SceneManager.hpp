#pragma once

#include "Scene.hpp"

class SceneManager : public Scene {
public:
    SceneManager();
    //SceneManager(const SceneManager&) = delete;
    //SceneManager& operator=(const SceneManager&) = delete;
    ~SceneManager();
private:
    Scene* mScene;
    //CommonData* const mCommonData;

public:
    bool Initialize();
    bool LoadData();

    bool SceneManagerUpdate();
    void draw();
    void input();
    void shutdown();

    Scene* update() override {
        return mScene;
    };
};