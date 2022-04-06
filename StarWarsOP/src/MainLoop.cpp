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

MainLoop::MainLoop()
	:mWindowWidth(1024),
	mWindowHeight(768),
	mIsRunning(true),
	mMoveSpeed(0.1),
	mMoveSensitivity(100.0f),
	mTextParam(0.25f),
	mInfH(20.0f),
	mInfL(100.0f)
{
	mTextDir = glm::vec3(0.0f, mInfL, mInfH);
	mTextPos = glm::vec3(0.0f, 0.0f, -mInfH);
	mTextDir = glm::normalize(mTextDir);
	mTextDir /= 7.0f;
}


bool MainLoop::Initialize()
{
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


	mWindow = SDL_CreateWindow("Wander OpenGL Tutorial", 100, 100,
		mWindowWidth, mWindowHeight, SDL_WINDOW_OPENGL);
	if (!mWindow)
	{
		SDL_Log("Failed to create window: %s", SDL_GetError());
		return false;
	}

	// Create an OpenGL context
	mContext = SDL_GL_CreateContext(mWindow);

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

	mAudioSystem = NULL;
	ERRCHECK(FMOD::Studio::System::create(&mAudioSystem));

	// The example Studio project is authored for 5.1 sound, so set up the system output mode to match
	FMOD::System* coreSystem = NULL;
	ERRCHECK(mAudioSystem->getCoreSystem(&coreSystem));
	ERRCHECK(coreSystem->setSoftwareFormat(0, FMOD_SPEAKERMODE_5POINT1, 0));

	ERRCHECK(mAudioSystem->initialize(1024, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, extraDriverData));


	if (!LoadShaders())
	{
		SDL_Log("Failed to load shaders.");
		return false;
	}


	//CreateSpriteVerts();

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
	// Compile Shader Program
	// Load TextShader
	{
		std::string vert_file = "./Shaders/Text.vert";
		std::string frag_file = "./Shaders/Text.frag";
		mTextShader = new Shader();
		if (!mTextShader->CreateShaderProgram(vert_file, frag_file)) {
			return false;
		}
	}

	mTextShader->UseProgram();
	{
		glm::mat4 spriteViewProj = glm::mat4(1.0f);
		spriteViewProj[0][0] = 2.0f / (float)mWindowWidth;
		spriteViewProj[1][1] = 2.0f / (float)mWindowHeight;
		spriteViewProj[3][2] = 1.0f;
		mTextShader->SetMatrixUniform("uViewProj", spriteViewProj);
	}

	// Load 3DTextShader
	{
		std::string vert_file = "./Shaders/3DText.vert";
		std::string frag_file = "./Shaders/3DText.frag";
		m3DTextShader = new Shader();
		if (!m3DTextShader->CreateShaderProgram(vert_file, frag_file)) {
			return false;
		}
	}

	mText = new Text(".\\resources\\arialuni.ttf", mTextShader, m3DTextShader);

	// 初期値を代入
 	mCameraUP = glm::vec3(0.0f, 0.0f, 1.0f);
	mCameraOrientation = glm::vec3(0, 0.5f, 0);
	glm::mat4 View = glm::lookAt(
		mCameraPos,
		mCameraPos + mCameraOrientation,
		mCameraUP);
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)mWindowWidth / mWindowHeight, 0.1f, 100.0f);

	m3DTextShader->UseProgram();
	m3DTextShader->SetMatrixUniform("uView", View);
	m3DTextShader->SetMatrixUniform("uProj", Projection);
	m3DTextShader->SetMatrixUniform("uTranslate", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 35.0f, 0.0f)));
	m3DTextShader->SetMatrixUniform("uRotate", glm::mat4(1.0f));



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
	{
		//std::vector<std::string> facesCubemap =
		//{
		//	"./resources/skybox/right.jpg",
		//	"./resources/skybox/left.jpg",
		//	"./resources/skybox/top.jpg",
		//	"./resources/skybox/bottom.jpg",
		//	"./resources/skybox/front.jpg",
		//	"./resources/skybox/back.jpg"
		//};
		std::vector<std::string> facesCubemap =
		{
			"./resources/StarWarsSkyBox/right.png",
			"./resources/StarWarsSkyBox/left.png",
			"./resources/StarWarsSkyBox/top.png",
			"./resources/StarWarsSkyBox/bottom.png",
			"./resources/StarWarsSkyBox/front.png",
			"./resources/StarWarsSkyBox/back.png"
		};


		mSkyBoxTexture = new Texture(facesCubemap);
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
		//Mesh* mesh = new Mesh("./resources/SimpleObj3/", "cubes.obj", mMeshShader, glm::vec3(0, -0.707, -0.707));
		//mesh->SetMeshPos(glm::vec3(0.0f, 35.0f, 0.0f));
		//mesh->SetMeshRotate(glm::mat4(1.0f));
		//mesh->SetMeshScale(1.0f);
		//mMeshes.insert(std::make_pair("SimpleObj", mesh));
	}

	{
		mTitlePos = 5.0f;
		Mesh* mesh = new Mesh("./resources/StarWars/", "StarWarsTitle.obj", mMeshShader, glm::vec3(0, -0.707, -0.707));
		mesh->SetMeshPos(glm::vec3(0.0f, mTitlePos, 0.0f));
		mesh->SetMeshRotate(glm::mat4(1.0f));
		mesh->SetMeshScale(1.0f);
		mMeshes.insert(std::make_pair("StarWarsTitle", mesh));
	}

	// Load Audio Bank
	FMOD::Studio::Bank* masterBank = NULL;
	ERRCHECK(mAudioSystem->loadBankFile(Common_MediaPath(".\\resources\\Master.bank"), FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank));

	FMOD::Studio::Bank* stringsBank = NULL;
	ERRCHECK(mAudioSystem->loadBankFile(Common_MediaPath(".\\resources\\Master.strings.bank"), FMOD_STUDIO_LOAD_BANK_NORMAL, &stringsBank));

	FMOD::Studio::EventDescription* BackMusicDesc = NULL;
	ERRCHECK(mAudioSystem->getEvent("event:/Main/StarWarsOP", &BackMusicDesc));
	mBackMusic = NULL;
	ERRCHECK(BackMusicDesc->createInstance(&mBackMusic));

	ERRCHECK(mBackMusic->start());	// Music Start!!

	return true;
}


void MainLoop::ProcessInput()
{
	SDL_Point mouse_position = { mWindowWidth / 2, mWindowHeight / 2 };
	SDL_GetMouseState(&mouse_position.x, &mouse_position.y);
	mMousePos.x = mouse_position.x;
	mMousePos.y = mouse_position.y;

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			mIsRunning = false;
			break;
		case SDL_MOUSEBUTTONDOWN:	// マウスのボタンが押されたら
		{
			//if (mPhase == PHASE_IDLE) {
			//	//mLastMousePos = mMousePos;
			//	//mSwipingDropPos = mMousePos / GRID_SIZE;
			//	mPhase = PHASE_MOVE;

			//	SDL_WarpMouseInWindow(mWindow, mWindowWidth / 2, mWindowHeight / 2);
			//	mMousePos.x = mWindowWidth / 2;
			//	mMousePos.y = mWindowHeight / 2;
			//	SDL_ShowCursor(SDL_DISABLE);
			//	std::cout << "----------------------------------------------called\n";
			//}
		}
		break;
		case SDL_MOUSEBUTTONUP:		// マウスを離したら
			//if (mPhase == PHASE_MOVE) {
			//	mPhase = PHASE_IDLE;

			//	/*if (EraseDrops()) {
			//		phase = PHASE_ERASE;
			//	}
			//	else {
			//		phase = PHASE_IDLE;
			//	}*/
			//	SDL_ShowCursor(SDL_ENABLE);
			//}
			break;
		}
	}

	const Uint8* keyState = SDL_GetKeyboardState(NULL);
	if (keyState[SDL_SCANCODE_ESCAPE] || keyState[SDL_SCANCODE_Q])	// escapeキーを押下すると終了
	{
		mIsRunning = false;
	}

	if (keyState[SDL_SCANCODE_W]) {
		mCameraPos += (float)mMoveSpeed * mCameraOrientation;
	}
	if (keyState[SDL_SCANCODE_S]) {
		mCameraPos -= (float)mMoveSpeed * mCameraOrientation;
	}
	if (keyState[SDL_SCANCODE_A]) {
		mCameraPos -= (float)mMoveSpeed * glm::normalize(glm::cross(mCameraOrientation, mCameraUP));
	}
	if (keyState[SDL_SCANCODE_D]) {
		mCameraPos += (float)mMoveSpeed * glm::normalize(glm::cross(mCameraOrientation, mCameraUP));
	}
}


void MainLoop::UpdateGame()
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

	//mCubeRotation = deltaTime * mCubeRotateVel * 10;

	//last = clock();
	//ComputeWorldTransform();	// Cubeのtransformを計算

	//if (mPhase == PHASE_MOVE) {
	//	//printf("%d %d\n", mMousePos.x, mMousePos.y);

	//	float rotX = mMoveSensitivity * (float)((float)mMousePos.y - ((float)mWindowHeight / 2.0f)) / (float)mWindowHeight;
	//	float rotY = mMoveSensitivity * (float)((float)mMousePos.x - ((float)mWindowWidth / 2.0f)) / (float)mWindowWidth;
	//	printf("rotX: %f rotY: %f\t", rotX, rotY);
	//	// Calculates upcoming vertical change in the Orientation
	//	glm::vec3 newOrientation = glm::rotate(mCameraOrientation, glm::radians(-rotX), glm::normalize(glm::cross(mCameraOrientation, mCameraUP)));

	//	// Decides whether or not the next vertical Orientation is legal or not
	//	int rad = abs(glm::angle(newOrientation, mCameraUP) - glm::radians(90.0f));
	//	std::cout << rad * 180 / M_PI << std::endl;
	//	if (abs(glm::angle(newOrientation, mCameraUP) - glm::radians(90.0f)) <= glm::radians(85.0f))
	//	{
	//		mCameraOrientation = newOrientation;
	//	}

	//	// Rotates the Orientation left and right
	//	mCameraOrientation = glm::rotate(mCameraOrientation, glm::radians(-rotY), mCameraUP);

	//	if ((mMousePos.x != mWindowWidth / 2) || (mMousePos.y != mWindowHeight / 2)) {
	//		SDL_WarpMouseInWindow(mWindow, mWindowWidth / 2, mWindowHeight / 2);
	//	}
	//}

	ERRCHECK(mAudioSystem->update());
}

void MainLoop::Draw()
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
		mSkyBoxShader->UseProgram();
		glm::mat4 skyView = glm::mat4(glm::mat3(glm::lookAt(mCameraPos, mCameraPos + mCameraOrientation, mCameraUP)));
		mSkyBoxShader->SetMatrixUniform("uView", skyView);
		glBindVertexArray(mSkyBoxVertexArray);
		mSkyBoxTexture->BindCubeMapTexture();
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		mSkyBoxTexture->UnBindTexture();
		glBindVertexArray(0);
	}
	glDepthFunc(GL_LESS);
	

	// --- draw sprites ---
	glDisable(GL_DEPTH_TEST);
	// Enable alpha blending on the color buffer
	glEnable(GL_BLEND);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	m3DTextShader->UseProgram();
	m3DTextShader->SetMatrixUniform("uView", CameraView);

	{
		//m3DTextShader->UseProgram();
		//m3DTextShader->SetMatrixUniform("uView", CameraView);
		//glm::mat4 textRotate = glm::rotate(glm::mat4(1.0f), (float)M_PI / 6.0f, glm::vec3(1.0, 0.0f, 0.0f));
		//Draw3DUTF(u"「ジェダイの文字列」", glm::vec3(0.0f, mTextPos, -3.0f), glm::vec3(1.0f, 1.0f, 1.0f), 0.03f, textRotate);
		//Draw3DUTF(u"「ジェダイの文字列」だよ", glm::vec3(0.0f, mTextPos - 3.0f, -3.0f), glm::vec3(1.0f, 1.0f, 1.0f), 0.03f, textRotate);
	}
	{
		//m3DTextShader->UseProgram();
		//m3DTextShader->SetMatrixUniform("uView", CameraView);
		float textRad = atan(mInfH / mInfL);
		glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), textRad, glm::vec3(1.0f, 0.0f, 0.0f));
		mText->Draw3DUTFText(TitleTelop.c_str(), mTextPos, glm::vec3(1.0f, 1.0f, 1.0f), 47.0f, 0.05f, rotate);
	}



	glBindVertexArray(0);

	SDL_GL_SwapWindow(mWindow);
}


void MainLoop::RunLoop()
{
	while (mIsRunning)
	{
		ProcessInput();
		UpdateGame();
		Draw();
	}
	ERRCHECK(mBackMusic->stop(FMOD_STUDIO_STOP_IMMEDIATE));
}

void MainLoop::Shutdown()
{
	delete mText;

	delete m3DTextShader;
	SDL_GL_DeleteContext(mContext);
	SDL_DestroyWindow(mWindow);
	SDL_Quit();
}