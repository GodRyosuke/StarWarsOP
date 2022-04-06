//#pragma once

#include"SceneManager.hpp"
#include "CommonData.hpp"
#include "glew.h"

#include <fstream>
class Title;
//class Game;

SceneManager::SceneManager()
	:Scene(new CommonData())
{
}


SceneManager::~SceneManager() {
	delete mScene;
	delete mCommonData;
}

bool SceneManager::Initialize()
{
	mCommonData->mWindowWidth = 1024;
	mCommonData->mWindowHeight = 768;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return false;
	}

	// Set OpenGL attributes
	// Use the core OpenGL profile
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	// Specify version 3.3
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	// Request a color buffer with 8-bits per RGBA channel
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	// Enable double buffering
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	// Force OpenGL to use hardware acceleration
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	mCommonData->mWindow = SDL_CreateWindow("Wander OpenGL Tutorial", 100, 100,
		mCommonData->mWindowWidth, mCommonData->mWindowHeight, SDL_WINDOW_OPENGL);
	if (!mCommonData->mWindow)
	{
		SDL_Log("Failed to create window: %s", SDL_GetError());
		return false;
	}

	// Create an OpenGL context
	mCommonData->mContext = SDL_GL_CreateContext(mCommonData->mWindow);

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		SDL_Log("Failed to initialize GLEW.");
		return false;
	}

	auto error_code = glGetError();

	setlocale(LC_CTYPE, "");
	// Audio System
	void* extraDriverData = NULL;
	Common_Init(&extraDriverData);

	mCommonData->mAudioSystem = NULL;
	ERRCHECK(FMOD::Studio::System::create(&mCommonData->mAudioSystem));

	// The example Studio project is authored for 5.1 sound, so set up the system output mode to match
	FMOD::System* coreSystem = NULL;
	ERRCHECK(mCommonData->mAudioSystem->getCoreSystem(&coreSystem));
	ERRCHECK(coreSystem->setSoftwareFormat(0, FMOD_SPEAKERMODE_5POINT1, 0));

	ERRCHECK(mCommonData->mAudioSystem->initialize(1024, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, extraDriverData));
	if (!LoadData()) {
		printf("error: failed to load data @ Scene Manager");
		return false;
	}


	// 最初のSceneを設定
	mScene = makeScene<Title>();

	return true;
}

bool SceneManager::LoadData()
{
	// Font読み込み
	mCommonData->mFont = TTF_OpenFont(".\\resources\\VL-Gothic-Regular.ttf", 128);

	// 日本語テキストデータ読み込み
	{
		std::string filePath = ".\\resources\\GameText.json";
		std::ifstream ifs(filePath.c_str());
		if (ifs.good()) {
			ifs >> mCommonData->mTextData;
		}
		else {
			printf("error: failed to load text lang data\n");
			return false;
		}
	}


	// デフォルトは英語
	mCommonData->mLangType = mCommonData->ENGLISH;


	// Load Master Bank
	FMOD::Studio::Bank* masterBank = NULL;
	ERRCHECK(mCommonData->mAudioSystem->loadBankFile(Common_MediaPath(".\\resources\\Master.bank"), FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank));

	FMOD::Studio::Bank* stringsBank = NULL;
	ERRCHECK(mCommonData->mAudioSystem->loadBankFile(Common_MediaPath(".\\resources\\Master.strings.bank"), FMOD_STUDIO_LOAD_BANK_NORMAL, &stringsBank));


	return true;
}

bool SceneManager::SceneManagerUpdate() {
	Scene* p = mScene->update();
	if (p == NULL) {    // ゲーム終了
		return false;
	}
	if (p != mScene) {
		delete mScene;
		mScene = p;
	}

	return true;
}



void SceneManager::draw() {
	mScene->draw();
}

void SceneManager::input()
{
	mScene->input();
}

void SceneManager::shutdown()
{
	mScene->shutdown();
	delete mScene;
}