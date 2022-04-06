#pragma once

#include <string>
#include <vector>
#include "glm.hpp"
#include "Shader.hpp"
#include <map>
#include "Texture.hpp"

class Mesh {
public:
	Mesh(std::string ObjFilePath, std::string MtlFilePath, Shader* shader, glm::vec3 LightDir);
	// Mesh のMaterial uniform値を設定
	void SetMeshLightings();
	// MeshのAffine変換をuniform値に反映
	void SetMeshTransforms();

	void BindVertexArray();

	int GetNumIndices() { return mNumIndices; }
	void Draw();
	void deDraw();
	void SetMeshPos(glm::vec3 pos) { mMeshPos = pos; }
	void SetMeshRotate(glm::mat4 rot) { mMeshRotate = rot; }
	void SetMeshScale(float scale) { mMeshScale = scale; }
	void SetMeshLightDirection(glm::vec3 dir) { mDirectionalLight.direction = dir; }

private:
	struct DirectionalLight {
		glm::vec3 direction;
		glm::vec3 diffuseColor;
		glm::vec3 specColor;
	};

	struct ObjSubSet {
		std::string MaterialName;
		int FaceStart;				// Index Bufferの開始Index
		unsigned int FaceCount;		// Index BufferのIndexの数
	};

	struct Material {
		glm::vec3 AmbientColor;
		DirectionalLight DirLight;
		float SpecPower;
		Texture* tex;
	};

	struct VAO {
		unsigned int VertexArray;
		unsigned int VertexBuffer;
		unsigned int IndexBuffer;
		int IndicesSize;
		std::string MaterialName;
	};


	std::map<std::string, Material> mMaterials;
	std::vector<ObjSubSet> mObjSubSets;
	std::vector<VAO> mVAOs;

	bool LoadObjFile(std::string FilePath, std::string ObjFileName);
	bool deLoadObjFile(std::string FilePath, std::string ObjFileName);
	bool LoadMaterials(std::string FilePath, std::string MtlFileName);
	bool LoadVAO(FILE* file, VAO& vao, int& VertexOffset, int& NormalOffset, int& IndexOffset);
	void CreateVAO(std::vector<float>VertexData, std::vector<float>NormalData,
		std::vector<float>UVData, std::vector<glm::ivec3> FaceVec, std::string mtlName,
		VAO& vao);

	unsigned int mVertexArray;
	unsigned int mVertexBuffer;
	unsigned int mIndexBuffer;
	int mNumIndices;

	std::vector<float> mVertices;
	std::vector<unsigned int> mIndices;

	glm::vec3 mLightDir;
	glm::vec3 mMeshPos;
	glm::mat4 mMeshRotate;
	float mMeshScale;
	Shader* mShader;

	// Lighting
	glm::vec3 mAmbientLightColor;
	DirectionalLight mDirectionalLight;
	float mSpecPower;

};

