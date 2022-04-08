#pragma once

#include "SceneManager.hpp"
#include <iostream>

class GameLoop {
public:
	GameLoop()
	{
		mSceneManager = new SceneManager();

		if (!mSceneManager->Initialize()) {
			printf("error: failed to initialize Scene Manager\n");
			return;
		}
	}
	void Run()
	{
		while (true) {
			mSceneManager->input();
			if (!mSceneManager->SceneManagerUpdate()) {
				break;
			}
			mSceneManager->draw();
		}

		mSceneManager->shutdown();
		int x = 0;
	}

private:
	SceneManager* mSceneManager;
};