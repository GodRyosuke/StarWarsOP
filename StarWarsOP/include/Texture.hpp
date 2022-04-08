#pragma once


#include <string>
#include "glew.h"
#include "stb_image.h"
#include <vector>

class Texture {
public:
	Texture();
	~Texture();
	Texture(std::string filePath);
	Texture(std::vector<std::string> filePaths);


	void BindTexture();
	void BindCubeMapTexture();
	void UnBindTexture();

	int getWidth() { return width; }
	int getHeight() { return height; }

private:

	int width;
	int height;
	GLuint texture_data;
};