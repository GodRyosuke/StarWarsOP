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

#include "Scene.hpp"
#include "CommonData.hpp"

#include FT_FREETYPE_H


class MainLoop : public Scene {
public:
	MainLoop(CommonData* const commonData);
	virtual ~MainLoop();
	virtual Scene* update() override;
	virtual void draw() override;
	virtual void input() override;
	virtual void shutdown() override;


	bool Initialize();
	//void RunLoop();

private:
	//void ProcessInput();
	//void UpdateGame();
	//void Draw();

	bool LoadData();
	bool LoadShaders();

	float mMoveSpeed;

	bool mIsRunning;

	Uint32 mTicksCount;


	// ---- Shaders ---
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

	float mTitlePos;
	glm::vec3 mTextDir;
	glm::vec3 mTextPos;

	float mTextParam;
	float mInfH;
	float mInfL;

	std::map<std::string, Mesh*> mMeshes;

	// ---- Sound Libraries ----
	FMOD::Studio::EventInstance* mBackMusic;
};

template<>
Scene* Scene::makeScene<MainLoop>() {
	return new MainLoop(mCommonData);
}