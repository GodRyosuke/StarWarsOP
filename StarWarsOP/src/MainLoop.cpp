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

	// FREE TYPE
	FT_Library library;
	//FT_Face face;
	FT_GlyphSlot slot;
	// Load Font
	FT_Init_FreeType(&library);
	//FT_New_Face(library, ".\\resources\\Carlito-Regular.ttf", 0, &mFontFace);
	FT_New_Face(library, ".\\resources\\arialuni.ttf", 0, &mFontFace);
	FT_Select_Charmap(mFontFace, ft_encoding_unicode);
	//FT_Select_Charmap(mFontFace, ft_encoding_sjis);
	//FT_Select_Charmap（m_face、FT_ENCODING_UNICODE）;
	FT_Set_Pixel_Sizes(mFontFace, 0, 48);
	slot = mFontFace->glyph;


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
	{
		std::string vert_file = "./Shaders/3DText.vert";
		std::string frag_file = "./Shaders/3DText.frag";
		m3DTextShader = new Shader();
		if (!m3DTextShader->CreateShaderProgram(vert_file, frag_file)) {
			return false;
		}
	}

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

	// Load 3D Text VAO
	m3DTextShader->UseProgram();
	glGenVertexArrays(1, &m3DTextVertexArray);
	glBindVertexArray(m3DTextVertexArray);

	glGenBuffers(1, &m3DTextVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m3DTextVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


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


	return true;
}


MainLoop::TexChar MainLoop::LoadUTFChar(char16_t c)
{
	if (FT_Load_Glyph(mFontFace, FT_Get_Char_Index(mFontFace, c), FT_LOAD_RENDER)) {
		std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
		exit(-1);
	}
	GLuint tex;
	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, tex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, mFontFace->glyph->bitmap.width, mFontFace->glyph->bitmap.rows,
		0, GL_RED, GL_UNSIGNED_BYTE, mFontFace->glyph->bitmap.buffer);
	//mFontWidth = mFontFace->glyph->bitmap.width;
	//mFontHeight = mFontFace->glyph->bitmap.rows;

	TexChar	 tc = {
		tex,
		glm::ivec2(mFontFace->glyph->bitmap.width, mFontFace->glyph->bitmap.rows),
		glm::ivec2(mFontFace->glyph->bitmap_left, mFontFace->glyph->bitmap_top),
		mFontFace->glyph->advance.x
	};

	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(tex, 0);		// unbind

	return tc;
}

void MainLoop::Draw3DUTF(std::u16string text, glm::vec3 pos, glm::vec3 color, float scale, glm::mat4 rot)
{
	m3DTextShader->UseProgram();
	glBindVertexArray(m3DTextVertexArray);

	glm::vec3 FontCenter = glm::vec3(0.0f);
	// 文字のtexcharの大きさを取得
	{
		int TexWidth = 0;
		int width = (mJapanTexChars.begin()->second.Advance >> 6) * scale;
		FontCenter.x = (width * text.length()) / 2.0f;
		FontCenter.y = width / 2.0f;
	}

	glm::mat4 SpriteTrans = glm::translate(glm::mat4(1.0f), pos - FontCenter);
	glm::mat4 SpriteRotate = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0, 0.0f, 1.0f));
	m3DTextShader->SetMatrixUniform("uTranslate", SpriteTrans);
	m3DTextShader->SetMatrixUniform("uRotate", rot);
	m3DTextShader->SetVectorUniform("textColor", color);


	glActiveTexture(GL_TEXTURE0);

	float x2 = 0;
	float y2 = 0;
	//float scale = 1.0f;
	const char16_t* str = text.c_str();
	for (int i = 0; str[i] != '\0'; i++) {
		auto itr = mJapanTexChars.find(str[i]);
		TexChar ch;
		if (itr == mJapanTexChars.end()) {		// まだ読み込まれていない文字なら
			ch = LoadUTFChar(str[i]);
			mJapanTexChars.insert(std::make_pair(str[i], ch));
		}
		else {
			ch = itr->second;
		}

		float xpos = x2 + ch.Bearing.x * scale;
		float ypos = y2 - (ch.Size.y - ch.Bearing.y) * scale;
		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;


		float textVertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};

		glBindTexture(GL_TEXTURE_2D, ch.texID);
		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, m3DTextVertexBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(textVertices), textVertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x2 += (ch.Advance >> 6) * scale;
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

void MainLoop::Draw3DUTFText(std::u16string text, glm::vec3 pos, glm::vec3 color, float textWidth, float scale, glm::mat4 rot)
{
	m3DTextShader->UseProgram();
	glBindVertexArray(m3DTextVertexArray);

	glm::vec3 FontCenter = glm::vec3(0.0f);
	float FontWidth = (mJapanTexChars.begin()->second.Advance >> 6) * scale;
	// 文字のtexcharの大きさを取得
	{
		FontCenter.x = textWidth / 2.0f;
		FontCenter.y = FontWidth / 2.0f;
	}
	FontWidth *= 1.2f;
	int maxRowChars = textWidth / FontWidth;	// 1行に入る最大文字数

	// 行数取得
	int maxRowCount = 1;
	int rowCharCount = 0;
	{
		const char16_t* str = text.c_str();
		for (int i = 0; i < str[i] != '\0'; i++) {
			if (str[i] == '\n') {
				maxRowCount++;
				continue;
			}
			else if (rowCharCount == maxRowChars) {
				maxRowCount++;
			}
		}
	}

	FontCenter.x = maxRowChars * FontWidth / 2.0f / 1.2;
	FontCenter.y = maxRowCount * FontWidth / 2.0f / 1.2;
	glm::mat4 SpriteTrans = glm::translate(glm::mat4(1.0f), pos - FontCenter);
	m3DTextShader->SetMatrixUniform("uTranslate", SpriteTrans);
	m3DTextShader->SetMatrixUniform("uRotate", rot);
	m3DTextShader->SetVectorUniform("textColor", color);



	glActiveTexture(GL_TEXTURE0);

	float x2 = 0;
	float y2 = 0;
	//float scale = 1.0f;
	const char16_t* str = text.c_str();
	rowCharCount = 0;
	for (int i = 0; str[i] != '\0'; i++) {
		if (str[i] == '\n') {
			x2 = 0.0f;
			y2 -= FontWidth;
			rowCharCount = 0;
			maxRowCount++;
			continue;
		}
		else if (rowCharCount == maxRowChars) {
			x2 = 0.0f;
			y2 -= FontWidth;
			rowCharCount = 0;
			maxRowCount++;
		}
		auto itr = mJapanTexChars.find(str[i]);
		TexChar ch;
		if (itr == mJapanTexChars.end()) {		// まだ読み込まれていない文字なら
			ch = LoadUTFChar(str[i]);
			mJapanTexChars.insert(std::make_pair(str[i], ch));
		}
		else {
			ch = itr->second;
		}

		float xpos = x2 + ch.Bearing.x * scale;
		float ypos = y2 - (ch.Size.y - ch.Bearing.y) * scale;
		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;


		float textVertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};

		glBindTexture(GL_TEXTURE_2D, ch.texID);
		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, m3DTextVertexBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(textVertices), textVertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x2 += (ch.Advance >> 6) * scale;
		rowCharCount++;
	}


	glBindTexture(GL_TEXTURE_2D, 0);
}


bool MainLoop::LoadData()
{
	// ---- Load Shaders -----
	if (!LoadShaders()) {
		std::cout << "Failed to Load Shaders\n";
		return false;
	}

	// 日本語文字列textures作成
	{
		const char16_t str[] = u"楠本崚介";
		for (int i = 0; str[i] != '\0'; i++) {
			mJapanTexChars.insert(std::make_pair(str[i], LoadUTFChar(str[i])));
		}
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
		m3DTextShader->UseProgram();
		m3DTextShader->SetMatrixUniform("uView", CameraView);
		float textRad = atan(mInfH / mInfL);
		glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), textRad, glm::vec3(1.0f, 0.0f, 0.0f));
		Draw3DUTFText(TitleTelop.c_str(), mTextPos, glm::vec3(1.0f, 1.0f, 1.0f), 47.0f, 0.05f, rotate);
		glm::mat4 testRotate = glm::rotate(glm::mat4(1.0f), (float)M_PI / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f));
		//Draw3DUTFText(TitleTelop.c_str(), glm::vec3(0.0f, 50.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 47.0f, 0.05f, testRotate);
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
	FT_Done_Face(mFontFace);

	delete m3DTextShader;
	SDL_GL_DeleteContext(mContext);
	SDL_DestroyWindow(mWindow);
	SDL_Quit();
}