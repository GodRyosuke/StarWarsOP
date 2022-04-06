#pragma once
#include "SDL.h"
#include "glew.h"
#include <iostream>
#include <string>
#include "ft2build.h"
#include "Shader.hpp"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include "gtx/rotate_vector.hpp"
#include "gtx/vector_angle.hpp"
#include <map>
#include "Texture.hpp"
#include "Mesh.hpp"
#include "fmod.hpp"
#include "fmod_studio.hpp"
#include "FMOD/common.hpp"
#include "Text.hpp"

#include FT_FREETYPE_H


class MainLoop {
public:
	MainLoop();
	bool Initialize();
	void RunLoop();
	void Shutdown();

private:
	void ProcessInput();
	void UpdateGame();
	void Draw();

	bool LoadData();
	bool LoadShaders();


	bool mIsRunning;

	SDL_Window* mWindow;
	// OpenGL context
	SDL_GLContext mContext;
	int mWindowWidth;
	int mWindowHeight;

	Text* mText;
	Uint32 mTicksCount;


	// ---- Shaders ---
	Shader* mTextShader;

	Shader* m3DTextShader;
	unsigned int m3DTextVertexArray;
	unsigned int m3DTextVertexBuffer;

	Shader* mSkyBoxShader;
	unsigned int mSkyBoxVertexArray;
	unsigned int mSkyBoxVertexBuffer;
	unsigned int mSkyBoxIndexBuffer;
	Texture* mSkyBoxTexture;

	Shader* mMeshShader;

	// Camera
	glm::vec3 mCameraPos;
	glm::vec3 mCameraUP;
	glm::vec3 mCameraOrientation;
	float mMoveSpeed;
	float mMoveSensitivity;

	glm::ivec3 mMousePos;

	float mTitlePos;
	glm::vec3 mTextDir;
	glm::vec3 mTextPos;

	float mTextParam;
	float mInfH;
	float mInfL;

	std::map<std::string, Mesh*> mMeshes;

	// ---- Sound Libraries ----
	FMOD::Studio::System* mAudioSystem;
	FMOD::Studio::EventInstance* mBackMusic;

};