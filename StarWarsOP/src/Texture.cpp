#include "Texture.hpp"

Texture::Texture()
{

}

Texture::Texture(std::string filePath)
{
	// Load from file
	int numColCh;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* PictureData = stbi_load(filePath.c_str(), &width, &height, &numColCh, 0);

	glGenTextures(1, &texture_data);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	auto colorCh = GL_RGBA;	// png—p
	if (numColCh == 3) {
		colorCh = GL_RGB;	// jpg—p
	}

	glTexImage2D(GL_TEXTURE_2D, 0, colorCh, width, height, 0, colorCh, GL_UNSIGNED_BYTE, PictureData);
	// Generates MipMaps
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(texture_data, 0);		// unbind
}

void Texture::BindTexture()
{
	glBindTexture(GL_TEXTURE_2D, texture_data);
}

void Texture::UnBindTexture()
{
	glBindTexture(texture_data, 0);
}