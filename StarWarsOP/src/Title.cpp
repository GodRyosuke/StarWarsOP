#include "Title.hpp"
#include "SDL.h"

class MainLoop;


Title::Title(CommonData* const commonData)
	:Scene(commonData),
	mIsRunning(true),
	mGotoGame(false),
	mOPTextAlpha(0.0f)
{
	if (!Initialize()) {
		printf("error: Failed to initialize Title Scene\n");
		exit(-1);
	}

	mPassedTime = clock();
}


Title::~Title()
{
	UnloadData();
}

bool Title::Initialize()
{
	mTicksCount = SDL_GetTicks();

	if (!LoadData()) {
		printf("error: failed to load data @ Title\n");
		return false;
	}

	return true;
}



bool Title::LoadData()
{

	return true;
}

void Title::shutdown()
{
	//IMG_Quit();
	//SDL_DestroyRenderer(mCommonData->mRenderer);
	//SDL_DestroyWindow(mCommonData->mWindow);
	//SDL_Quit();
}

void Title::UnloadData()
{
	//SDL_DestroyTexture(mTitleTexture);
}

Scene* Title::update()
{
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16))
		;

	float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}
	mTicksCount = SDL_GetTicks();



	if (!mIsRunning) {
		return nullptr;
	}
	if (mGotoGame) {
		Scene* game = makeScene<MainLoop>();
		return game;
	}

	int thisTime = clock();;
	if ((thisTime - mPassedTime) > 6000) {
		mGotoGame = true;
	}

	if (((thisTime - mPassedTime) > 500) && ((thisTime - mPassedTime) < 2000)) {	// 起動して1秒経過したら文字表示
		if (mOPTextAlpha < 1.0f) {
			mOPTextAlpha += 0.05f;
		} 
	}

	if ((thisTime - mPassedTime) > 5000) {
		if (mOPTextAlpha > 0.0f) {
			mOPTextAlpha -= 0.05f; 
			std::cout << mOPTextAlpha << std::endl;
		}
	}

	ERRCHECK(mCommonData->mAudioSystem->update());

	return this;
}

void Title::draw()
{
	glClearColor(0, 0, 0.1, 1.0f);
	// Clear the color buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw mesh components
	// Enable depth buffering/disable alpha blend
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);



	// --- draw sprites ---
	glDisable(GL_DEPTH_TEST);
	// Enable alpha blending on the color buffer
	glEnable(GL_BLEND);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

	mCommonData->mText->DrawUTFText(u"遠い昔　はるかかなたの\n銀河系で・・・", glm::vec3(0.0f), glm::vec3(0.0f, 0.75f, 1.0f), 11, 1.0f, 0.0f, mOPTextAlpha);


	//std::cout << clock() << std::endl;

	glBindVertexArray(0);

	SDL_GL_SwapWindow(mCommonData->mWindow);
}

void Title::input()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			mIsRunning = false;
			break;
		}
	}

	const Uint8* keyState = SDL_GetKeyboardState(NULL);
	if (keyState[SDL_SCANCODE_ESCAPE] || keyState[SDL_SCANCODE_Q])	// escapeもしくはqキーを押下すると終了
	{
		mIsRunning = false;
	}
	if (keyState[SDL_SCANCODE_RETURN]) {	// Enter キーでタイトル移動
		mGotoGame = true;
	}
}