#pragma once

#include <stdlib.h>
#include <iostream>
#include"Scene.hpp"
#include "CommonData.hpp"
#include <time.h>

class Title : public Scene {
public:
	Title(CommonData* const commonData);
	virtual ~Title();
	virtual Scene* update() override;
	virtual void draw() override;
	virtual void input() override;
	virtual void shutdown() override;

private:
	bool Initialize();
	bool LoadData();	// ゲームで使用するデータのロード
	void UnloadData();

	bool mIsRunning;
	bool mGotoGame;

	unsigned int mPassedTime;
	float mOPTextAlpha;

	Uint32 mTicksCount;
};

template<>
Scene* Scene::makeScene<Title>() {
	return new Title(mCommonData);
}