#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION

#include "MainLoop.hpp"

static std::u16string TitleTelop = u"\
エピソード４\n\
新たな希望\n\
\n\
時は内乱の嵐が吹き荒れるさなか。凶悪な銀河帝国の支配に対し\
反乱軍の宇宙艦隊は秘密基地から\
奇襲攻撃を仕掛け、初めて勝利を手にした。\n\
\n\
その戦闘の間に、反乱軍のスパイは帝国の究極兵器に関する秘密のの設計図\
を盗み出すことに成功した。それは「デス･スター」と呼ばれ惑星を丸ごと粉砕できる\
破壊力を兼ね備えた恐るべき武装宇宙要塞だった。\n\
\n\
邪悪な敵国の手先どもに追われながらも、レイア姫は自らの宇宙船を駆って、\
盗み出した設計図を携え故郷の星への道を急いでいた。\
この設計図こそが、人民を救い、再び銀河系に自由を\
取り戻すための鍵となるのだ・・・\
";

float skyboxVertices[] =
{
	//   Coordinates
	-1.0f, -1.0f,  1.0f,//        7--------6
	 1.0f, -1.0f,  1.0f,//       /|       /|
	 1.0f, -1.0f, -1.0f,//      4--------5 |
	-1.0f, -1.0f, -1.0f,//      | |      | |
	-1.0f,  1.0f,  1.0f,//      | 3------|-2
	 1.0f,  1.0f,  1.0f,//      |/       |/
	 1.0f,  1.0f, -1.0f,//      0--------1
	-1.0f,  1.0f, -1.0f
};

unsigned int skyboxIndices[] =
{
	// Right
	1, 2, 6,
	6, 5, 1,
	// Left
	0, 4, 7,
	7, 3, 0,
	// Top
	4, 5, 6,
	6, 7, 4,
	// Bottom
	0, 3, 2,
	2, 1, 0,
	// Back
	0, 1, 5,
	5, 4, 0,
	// Front
	3, 7, 6,
	6, 2, 3
};

MainLoop::MainLoop(CommonData* const commonData)
	:Scene(commonData), 
	mIsRunning(true),
	mTextParam(0.25f),
	mInfH(20.0f),
	mInfL(100.0f),
	mMoveSpeed(0.1f), 
	isSky(false)
{
	mTextDir = glm::vec3(0.0f, mInfL, mInfH);
	mTextPos = glm::vec3(0.0f, 0.0f, -mInfH);
	mTextDir = glm::normalize(mTextDir);
	mTextDir /= 7.0f;

	if (!Initialize()) {
		printf("error: Failed to initialize Title Scene\n");
		exit(-1);
	}
}

MainLoop::~MainLoop()
{

}


bool MainLoop::Initialize()
{
	if (!LoadData())
	{
		SDL_Log("Failed to load data.");
		return false;
	}


	mTicksCount = SDL_GetTicks();

	return true;
}

bool MainLoop::LoadShaders()
{
	// Camera SetUP
 	mCameraUP = glm::vec3(0.0f, 0.0f, 1.0f);
	mCameraOrientation = glm::vec3(0, 0.5f, 0);
	glm::mat4 View = glm::lookAt(
		mCameraPos,
		mCameraPos + mCameraOrientation,
		mCameraUP);
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)mCommonData->mWindowWidth / mCommonData->mWindowHeight, 0.1f, 100.0f);

	mCommonData->m3DTextShader->UseProgram();
	mCommonData->m3DTextShader->SetMatrixUniform("uView", View);
	mCommonData->m3DTextShader->SetMatrixUniform("uProj", Projection);
	mCommonData->m3DTextShader->SetMatrixUniform("uTranslate", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 35.0f, 0.0f)));
	mCommonData->m3DTextShader->SetMatrixUniform("uRotate", glm::mat4(1.0f));



	// Load Mesh Shader
	{
		std::string vert_file = "./Shaders/Mesh.vert";
		std::string frag_file = "./Shaders/Mesh.frag";
		mMeshShader = new Shader();
		if (!mMeshShader->CreateShaderProgram(vert_file, frag_file)) {
			return false;
		}
	}
	mMeshShader->UseProgram();
	mMeshShader->SetMatrixUniform("uView", View);
	mMeshShader->SetMatrixUniform("uProj", Projection);
	mMeshShader->SetMatrixUniform("uTranslate", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 35.0f, 0.0f)));
	mMeshShader->SetMatrixUniform("uRotate", glm::mat4(1.0f));
	mMeshShader->SetVectorUniform("uCameraPos", mCameraPos);


	// Load SkyBoxShader
	{
		std::string vert_file = "./Shaders/SkyBox.vert";
		std::string frag_file = "./Shaders/SkyBox.frag";
		mSkyBoxShader = new Shader();
		if (!mSkyBoxShader->CreateShaderProgram(vert_file, frag_file)) {
			return false;
		}
	}
	mSkyBoxShader->UseProgram();
	mSkyBoxShader->SetMatrixUniform("uVew", View);
	mSkyBoxShader->SetMatrixUniform("uProj", Projection);
	{
		glm::mat4 rot = glm::rotate(glm::mat4(1.0f), (float)M_PI / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f));
		mSkyBoxShader->SetMatrixUniform("uRot", rot);
	}
	// Load SkyBox VAO
	glGenVertexArrays(1, &mSkyBoxVertexArray);
	glBindVertexArray(mSkyBoxVertexArray);

	glGenBuffers(1, &mSkyBoxVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mSkyBoxVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

	glGenBuffers(1, &mSkyBoxIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mSkyBoxIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), skyboxIndices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, mSkyBoxVertexBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// unbind cube vertex arrays
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Sky BoxのTextureを読み込む
	// Load Sky Skybox
	{
		std::vector<std::string> facesCubemap =
		{
			"./resources/skybox/right.jpg",
			"./resources/skybox/left.jpg",
			"./resources/skybox/top.jpg",
			"./resources/skybox/bottom.jpg",
			"./resources/skybox/front.jpg",
			"./resources/skybox/back.jpg"
		};
		mSkyBoxTexture = new Texture(facesCubemap);
	}
	{
		std::vector<std::string> facesCubemap =
		{
			"./resources/StarWarsSkyBox/right.png",
			"./resources/StarWarsSkyBox/left.png",
			"./resources/StarWarsSkyBox/top.png",
			"./resources/StarWarsSkyBox/bottom.png",
			"./resources/StarWarsSkyBox/front.png",
			"./resources/StarWarsSkyBox/back.png"
		};


		mSpaceBoxTexture = new Texture(facesCubemap);
	}


	return true;
}



bool MainLoop::LoadData()
{
	// ---- Load Shaders -----
	if (!LoadShaders()) {
		std::cout << "Failed to Load Shaders\n";
		return false;
	}


	// Mesh読み込み
	{
		mTitlePos = 5.0f;
		Mesh* mesh = new Mesh("./resources/StarWars/", "StarWarsTitle.obj", mMeshShader, glm::vec3(0, -0.707, -0.707));
		mesh->SetMeshPos(glm::vec3(0.0f, mTitlePos, 0.0f));
		mesh->SetMeshRotate(glm::mat4(1.0f));
		mesh->SetMeshScale(1.0f);
		mMeshes.insert(std::make_pair("StarWarsTitle", mesh));
	}

	// Load Audio Bank
	FMOD::Studio::EventDescription* BackMusicDesc = NULL;
	ERRCHECK(mCommonData->mAudioSystem->getEvent("event:/Main/StarWarsOP", &BackMusicDesc));
	mBackMusic = NULL;
	ERRCHECK(BackMusicDesc->createInstance(&mBackMusic));

	ERRCHECK(mBackMusic->start());	// Music Start!!

	return true;
}


void MainLoop::input()
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
	if (keyState[SDL_SCANCODE_ESCAPE] || keyState[SDL_SCANCODE_Q])	// escapeキーを押下すると終了
	{
		mIsRunning = false;
	}

	if (keyState[SDL_SCANCODE_W]) {
		mCameraPos += mMoveSpeed * mCameraOrientation;
	}
	if (keyState[SDL_SCANCODE_S]) {
		mCameraPos -= mMoveSpeed * mCameraOrientation;
	}
	if (keyState[SDL_SCANCODE_A]) {
		mCameraPos -= mMoveSpeed * glm::normalize(glm::cross(mCameraOrientation, mCameraUP));
	}
	if (keyState[SDL_SCANCODE_D]) {
		mCameraPos += mMoveSpeed * glm::normalize(glm::cross(mCameraOrientation, mCameraUP));
	}

	// K, Pキーでsky box変更
	if (keyState[SDL_SCANCODE_K]) {
		isSky = true;
	}
	if (keyState[SDL_SCANCODE_P]) {
		isSky = false;
	}
}


Scene* MainLoop::update()
{
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16))
		;

	float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}
	mTicksCount = SDL_GetTicks();

	mTitlePos += 0.07f;
	mMeshes["StarWarsTitle"]->SetMeshPos(glm::vec3(0.0f, mTitlePos, 0.0f));

	mTextParam += 0.0001f;
	mTextPos = mTextParam * mTextDir + mTextPos;

	if (!mIsRunning) {
		return nullptr;
	}


	ERRCHECK(mCommonData->mAudioSystem->update());

	return this;
}

void MainLoop::draw()
{
	glClearColor(0, 0, 0.1, 1.0f);
	// Clear the color buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw mesh components
	// Enable depth buffering/disable alpha blend
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glm::mat4 CameraView = glm::lookAt(
		mCameraPos,
		mCameraPos + mCameraOrientation,
		mCameraUP
	);
	mMeshShader->UseProgram();
	mMeshShader->SetMatrixUniform("uView", CameraView);
	mMeshShader->SetVectorUniform("uCameraPos", mCameraPos);
	{
		//glm::mat4 rot = glm::rotate(glm::mat4(1.0f), (float)M_PI / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f));
		//mMeshes["SimpleObj"]->SetMeshRotate(rot);
	}
	{
		glm::mat4 rot = glm::rotate(glm::mat4(1.0f), (float)M_PI, glm::vec3(1.0f, 0.0f, 0.0f));
		mMeshes["StarWarsTitle"]->SetMeshRotate(rot);
	}

	// Draw Mesh
	for (auto itr = mMeshes.begin(); itr != mMeshes.end(); itr++) {
		itr->second->Draw();
	}

	glDepthFunc(GL_LEQUAL);
	{
		Texture* skyBoxTexture = mSpaceBoxTexture;
		if (isSky) {
			skyBoxTexture = mSkyBoxTexture;
		}

		mSkyBoxShader->UseProgram();
		glm::mat4 skyView = glm::mat4(glm::mat3(glm::lookAt(mCameraPos, mCameraPos + mCameraOrientation, mCameraUP)));
		mSkyBoxShader->SetMatrixUniform("uView", skyView);
		glBindVertexArray(mSkyBoxVertexArray);
		skyBoxTexture->BindCubeMapTexture();
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		skyBoxTexture->UnBindTexture();
		glBindVertexArray(0);
	}
	glDepthFunc(GL_LESS);
	

	// --- draw sprites ---
	glDisable(GL_DEPTH_TEST);
	// Enable alpha blending on the color buffer
	glEnable(GL_BLEND);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	mCommonData->m3DTextShader->UseProgram();
	mCommonData->m3DTextShader->SetMatrixUniform("uView", CameraView);
	{
		//m3DTextShader->UseProgram();
		//m3DTextShader->SetMatrixUniform("uView", CameraView);
		float textRad = atan(mInfH / mInfL);
		glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), textRad, glm::vec3(1.0f, 0.0f, 0.0f));
		mCommonData->mText->Draw3DUTFText(TitleTelop.c_str(), mTextPos, glm::vec3(1.0f, 1.0f, 1.0f), 16, 0.05f, rotate);
	}

	//mCommonData->mText->Draw3DUTFText(u"これは文字列です\n文字列", glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(0.2f, 1.0f, 0.2f),
	//	20.0f, 0.05f, glm::rotate(glm::mat4(1.0f), (float)M_PI / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f)));



	glBindVertexArray(0);

	SDL_GL_SwapWindow(mCommonData->mWindow);
}


//void MainLoop::RunLoop()
//{
//	while (mIsRunning)
//	{
//		ProcessInput();
//		UpdateGame();
//		Draw();
//	}
//	ERRCHECK(mBackMusic->stop(FMOD_STUDIO_STOP_IMMEDIATE));
//}


void MainLoop::UnLoadData()
{
	delete mSkyBoxTexture;
	delete mSpaceBoxTexture;
	delete mSkyBoxShader;
	delete mMeshShader;
}

void MainLoop::shutdown()
{
	UnLoadData();

	//delete mText;

	//delete m3DTextShader;
	//SDL_GL_DeleteContext(mContext);
	//SDL_DestroyWindow(mWindow);
	//SDL_Quit();
}