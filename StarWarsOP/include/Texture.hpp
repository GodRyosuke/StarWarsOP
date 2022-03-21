#pragma once


#include <string>
#include "glew.h"
#include "stb_image.h"

class Texture {
public:
	Texture();
	Texture(std::string filePath);


	void BindTexture();
	void UnBindTexture();

	int getWidth() { return width; }
	int getHeight() { return height; }

private:

	int width;
	int height;
	GLuint texture_data;
};