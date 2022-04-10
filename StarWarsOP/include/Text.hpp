#pragma once

#include <string>
#include <map>
#include "Shader.hpp"
#include "ft2build.h"
#include FT_FREETYPE_H


class Text {
public:
	Text(std::string TextVert, std::string TextFrag,
		std::string Text3DVert, std::string Text3DFrag);

	Text(std::string ttfPath, Shader* textShader,Shader* text3DShader);
	~Text();

	// 2D•`‰æ
	void DrawUTF(std::u16string text, glm::vec3 pos, glm::vec3 color, float scale = 1.0f, float rot = 0.0f, float textAlpha = 1.0f);
	void DrawUTFText(std::u16string text, glm::vec3 pos, glm::vec3 color, int maxCharCount, float scale = 1.0f, float rot = 0.0f, float textAlpha = 1.0f);

	// 3D•`‰æ
	void Draw3DUTF(std::u16string text, glm::vec3 pos, glm::vec3 color, float scale = 1.0f, glm::mat4 rot = glm::mat4(1.0f), float textAlpha = 1.0f);
	void Draw3DUTFText(std::u16string text, glm::vec3 pos, glm::vec3 color, int maxRowCount, float scale = 1.0f, glm::mat4 rot = glm::mat4(1.0f), float textAlpha = 1.0f);


private:
	struct TexChar {
		GLuint texID;
		glm::ivec2 Size;
		glm::ivec2 Bearing;
		unsigned int Advance;
	};

	TexChar LoadUTFChar(char16_t c);

	unsigned int mTextVertexArray;
	unsigned int mTextVertexBuffer;
	unsigned int m3DTextVertexArray;
	unsigned int m3DTextVertexBuffer;


	FT_Face mFontFace;
	Shader* m3DTextShader;
	Shader* mTextShader;
	std::map<char16_t, TexChar> mJapanTexChars;

};