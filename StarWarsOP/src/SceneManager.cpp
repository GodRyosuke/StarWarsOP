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


	// Å‰‚ÌScene‚ğİ’è
	mScene = makeScene<Title>();

	return true;
}

bool SceneManager::LoadData()
{
	// TextShader“Ç‚İ‚İ
	{
		std::string vert_file = "./Shaders/Text.vert";
		std::string frag_file = "./Shaders/Text.frag";
		mCommonData->mTextShader = new Shader();
		if (!mCommonData->mTextShader->CreateShaderProgram(vert_file, frag_file)) {
			return false;
		}
	}

	mCommonData->mTextShader->UseProgram();
	{
		glm::mat4 spriteViewProj = glm::mat4(1.0f);
		spriteViewProj[0][0] = 2.0f / (float)mCommonData->mWindowWidth;
		spriteViewProj[1][1] = 2.0f / (float)mCommonData->mWindowHeight;
		spriteViewProj[3][2] = 1.0f;
		mCommonData->mTextShader->SetMatrixUniform("uViewProj", spriteViewProj);
	}

	// 3DTextShader“Ç‚İ‚İ
	{
		std::string vert_file = "./Shaders/3DText.vert";
		std::string frag_file = "./Shaders/3DText.frag";
		mCommonData->m3DTextShader = new Shader();
		if (!mCommonData->m3DTextShader->CreateShaderProgram(vert_file, frag_file)) {
			return false;
		}
	}


	// Font“Ç‚İ‚İ
	mCommonData->mText = new Text(".\\resources\\arialuni.ttf", mCommonData->mTextShader, mCommonData->m3DTextShader);



	// Load Master Bank
	FMOD::Studio::Bank* masterBank = NULL;
	ERRCHECK(mCommonData->mAudioSystem->loadBankFile(Common_MediaPath(".\\resources\\Master.bank"), FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank));

	FMOD::Studio::Bank* stringsBank = NULL;
	ERRCHECK(mCommonData->mAudioSystem->loadBankFile(Common_MediaPath(".\\resources\\Master.strings.bank"), FMOD_STUDIO_LOAD_BANK_NORMAL, &stringsBank));


	return true;
}

bool SceneManager::SceneManagerUpdate() {
	Scene* p = mScene->update();
	if (p == NULL) {    // ƒQ[ƒ€I—¹
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

	// CommonData‚ÌUnload
	delete mCommonData->mText;

	SDL_GL_DeleteContext(mCommonData->mContext);
	SDL_DestroyWindow(mCommonData->mWindow);
	SDL_Quit();
}