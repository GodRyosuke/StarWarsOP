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

#include FT_FREETYPE_H


class MainLoop {
public:
	MainLoop();
	bool Initialize();
	void RunLoop();
	void Shutdown();

private:
	struct TexChar {
		GLuint texID;
		glm::ivec2 Size;
		glm::ivec2 Bearing;
		unsigned int Advance;
	};

	void ProcessInput();
	void UpdateGame();
	void Draw();

	bool LoadData();
	bool LoadShaders();
	TexChar LoadUTFChar(char16_t c);
	void Draw3DUTF(std::u16string text, glm::vec3 pos, glm::vec3 color, float scale = 1.0f, glm::mat4 rot = glm::mat4(1.0f));
	void Draw3DUTFText(std::u16string text, glm::vec3 pos, glm::vec3 color, float textWidth, float scale = 1.0f, glm::mat4 rot = glm::mat4(1.0f));


	bool mIsRunning;

	SDL_Window* mWindow;
	// OpenGL context
	SDL_GLContext mContext;
	int mWindowWidth;
	int mWindowHeight;

	FT_Face mFontFace;
	Uint32 mTicksCount;
	std::map<char16_t, TexChar> mJapanTexChars;


	// ---- Shaders ---
	Shader* m3DTextShader;
	unsigned int m3DTextVertexArray;
	unsigned int m3DTextVertexBuffer;

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

	std::map<std::string, Mesh*> mMeshes;


};