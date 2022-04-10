#pragma once

class CommonData;

class Scene {
protected:
    //コンストラクタ
    //Scene() {}
    Scene(CommonData* const commonData) :
        mCommonData(commonData)
    {}

public:
    //コピー禁止
    Scene(const Scene&) = delete;
    //コピー禁止
    Scene& operator=(const Scene&) = delete;

    //仮想関数化したデストラクタ
    virtual ~Scene() = default;

public:
    virtual Scene* update() = 0;
    virtual void draw() = 0;
    virtual void input() = 0;
    virtual void shutdown() = 0;

protected:
    CommonData* const mCommonData;

    //実体生成用関数
    template<typename T>
    Scene* makeScene();
};