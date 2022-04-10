#include "Text.hpp"
#include <iostream>

//Text::Text(std::string TextVert, std::string TextFrag,
//	std::string Text3DVert, std::string Text3DFrag)
//{
//	m3DTextShader = new Shader();
//	if (!m3DTextShader->CreateShaderProgram(Text3DVert, Text3DFrag)) {
//		return;
//	}
//}

Text::Text(std::string ttfPath, Shader* textShader, Shader* text3DShader)
{
	FT_Library library;
	//FT_Face face;
	FT_GlyphSlot slot;
	// Load Font
	FT_Init_FreeType(&library);
	//FT_New_Face(library, ".\\resources\\Carlito-Regular.ttf", 0, &mFontFace);
	FT_New_Face(library,ttfPath.c_str(), 0, &mFontFace);
	FT_Select_Charmap(mFontFace, ft_encoding_unicode);
	//FT_Select_Charmap(mFontFace, ft_encoding_sjis);
	//FT_Select_Charmap（m_face、FT_ENCODING_UNICODE）;
	FT_Set_Pixel_Sizes(mFontFace, 0, 48);
	slot = mFontFace->glyph;

	// Create Text VAO
	mTextShader = textShader;
	mTextShader->UseProgram();
	glGenVertexArrays(1, &mTextVertexArray);
	glBindVertexArray(mTextVertexArray);

	glGenBuffers(1, &mTextVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mTextVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	// Create 3DText VAO
	text3DShader->UseProgram();
	m3DTextShader = text3DShader;
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


	// 初期値を入れる
	{
		const char16_t str[] = u"楠本崚介";
		for (int i = 0; str[i] != '\0'; i++) {
			mJapanTexChars.insert(std::make_pair(str[i], LoadUTFChar(str[i])));
		}
	}
}

Text::~Text()
{
	FT_Done_Face(mFontFace);
	delete m3DTextShader;
	delete mTextShader;
}



Text::TexChar Text::LoadUTFChar(char16_t c)
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

void Text::DrawUTF(std::u16string text, glm::vec3 pos, glm::vec3 color, float scale, float rot, float textAlpha)
{
	mTextShader->UseProgram();
	glBindVertexArray(mTextVertexArray);

	glm::vec3 FontCenter = glm::vec3(0.0f);
	// 文字のtexcharの大きさを取得
	{
		int TexWidth = 0;
		int width = (mJapanTexChars[u'楠'].Advance >> 6) * scale;
		FontCenter.x = (width * text.length()) / 2.0f;
		FontCenter.y = width / 2.0f;
	}

	glm::mat4 SpriteTrans = glm::translate(glm::mat4(1.0f), pos - FontCenter);
	glm::mat4 SpriteRotate = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0, 0.0f, 1.0f));
	mTextShader->SetMatrixUniform("uWorldTransform", SpriteTrans);
	mTextShader->SetMatrixUniform("uRotate", SpriteRotate);
	mTextShader->SetVectorUniform("textColor", color);
	mTextShader->SetFloatUniform("uTextAlpha", textAlpha);


	glActiveTexture(GL_TEXTURE0);

	int x2 = 0;
	int y2 = 0;
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
		glBindBuffer(GL_ARRAY_BUFFER, mTextVertexBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(textVertices), textVertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x2 += (ch.Advance >> 6) * scale;
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Text::DrawUTFText(std::u16string text, glm::vec3 pos, glm::vec3 color, int maxRowChars, float scale, float rot, float textAlpha)
{
	mTextShader->UseProgram();
	glBindVertexArray(mTextVertexArray);

	glm::vec3 FontCenter = glm::vec3(0.0f);
	//float FontWidth = (mJapanTexChars.begin()->second.Advance >> 6) * scale;
	float FontWidth = (mJapanTexChars[u'楠'].Advance >> 6) * scale;
	float FontHeight = (mJapanTexChars[u'楠'].Size.y >> 6) * scale;
	auto this_begin = mJapanTexChars.begin();
	// 文字のtexcharの大きさを取得
	//FontWidth *= 1.2f;
	//int maxRowChars = textWidth / FontWidth;	// 1行に入る最大文字数

	FontCenter.x = maxRowChars * FontWidth / 2.0f;

	// 行数取得
	int maxRowCount = 1;
	int rowCharCount = 1;
	{
		const char16_t* str = text.c_str();
		for (int i = 0; str[i] != '\0'; i++) {
			if (str[i] == u'\n') {
				maxRowCount++;
				rowCharCount = 1;
				continue;
			}
			else if (rowCharCount == maxRowChars) {
				maxRowCount++;
				rowCharCount = 0;
			}

			rowCharCount++;
		}
	}
	FontCenter.y = -maxRowCount * FontHeight / 2.0f;

	glm::mat4 SpriteTrans = glm::translate(glm::mat4(1.0f), pos - FontCenter);
	glm::mat4 SpriteRotate = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0, 0.0f, 1.0f));
	mTextShader->SetMatrixUniform("uWorldTransform", SpriteTrans);
	mTextShader->SetMatrixUniform("uRotate", SpriteRotate);
	mTextShader->SetVectorUniform("textColor", color);
	mTextShader->SetFloatUniform("uTextAlpha", textAlpha);


	glActiveTexture(GL_TEXTURE0);

	int x2 = 0;
	int y2 = 0;
	//float scale = 1.0f;
	const char16_t* str = text.c_str();
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
		glBindBuffer(GL_ARRAY_BUFFER, mTextVertexBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(textVertices), textVertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x2 += (ch.Advance >> 6) * scale;
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}


void Text::Draw3DUTF(std::u16string text, glm::vec3 pos, glm::vec3 color, float scale, glm::mat4 rot, float textAlpha)
{
	m3DTextShader->UseProgram();
	glBindVertexArray(m3DTextVertexArray);

	glm::vec3 FontCenter = glm::vec3(0.0f);
	// 文字のtexcharの大きさを取得
	{
		int TexWidth = 0;
		int width = (mJapanTexChars[u'楠'].Advance >> 6) * scale;
		FontCenter.x = (width * text.length()) / 2.0f;
		FontCenter.y = width / 2.0f;
	}

	glm::mat4 SpriteTrans = glm::translate(glm::mat4(1.0f), pos - FontCenter);
	glm::mat4 SpriteRotate = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0, 0.0f, 1.0f));
	m3DTextShader->SetMatrixUniform("uTranslate", SpriteTrans);
	m3DTextShader->SetMatrixUniform("uRotate", rot);
	m3DTextShader->SetVectorUniform("textColor", color);
	m3DTextShader->SetFloatUniform("uTextAlpha", textAlpha);


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

void Text::Draw3DUTFText(std::u16string text, glm::vec3 pos, glm::vec3 color, int maxRowChars, float scale, glm::mat4 rot, float textAlpha)
{
	m3DTextShader->UseProgram();
	glBindVertexArray(m3DTextVertexArray);

	glm::vec3 FontCenter = glm::vec3(0.0f);
	//float FontWidth = (mJapanTexChars.begin()->second.Advance >> 6) * scale;
	float FontWidth = (mJapanTexChars[u'楠'].Advance >> 6) * scale;
	float FontHeight = (mJapanTexChars[u'楠'].Size.y >> 6) * scale;

	auto this_begin = mJapanTexChars.begin();
	// 文字のtexcharの大きさを取得
	FontWidth *= 1.2f;

	//int maxRowChars = textWidth / FontWidth;	// 1行に入る最大文字数


	// 行数取得
	int maxRowCount = 1;
	int rowCharCount = 1;
	{
		const char16_t* str = text.c_str();
		for (int i = 0; str[i] != '\0'; i++) {
			if (str[i] == '\n') {
				maxRowCount++;
				rowCharCount = 1;
				continue;
			}
			else if (rowCharCount == maxRowChars) {
				maxRowCount++;
				rowCharCount = 0;
			}

			rowCharCount++;
		}
	}
	

	FontCenter.x = maxRowChars * FontWidth / 2.0f / 1.2f;
	FontCenter.y = -maxRowCount * FontHeight / 2.0f;
	glm::mat4 SpriteTrans = glm::translate(glm::mat4(1.0f), pos - FontCenter);
	m3DTextShader->SetMatrixUniform("uTranslate", SpriteTrans);
	m3DTextShader->SetMatrixUniform("uRotate", rot);
	m3DTextShader->SetVectorUniform("textColor", color);
	m3DTextShader->SetFloatUniform("uTextAlpha", textAlpha);




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


