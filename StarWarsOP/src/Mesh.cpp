#define _CRT_SECURE_NO_WARNINGS

#include "Mesh.hpp"
#include <iostream>


// 1objにつき、1mtlの形式のみ対応


Mesh::Mesh(std::string FilePath, std::string ObjFileName, Shader* shader, glm::vec3 LightDir)
	:mShader(shader),
	mVertices(0),
	mIndices(0),
	mLightDir(LightDir)
{

	if (!LoadObjFile(FilePath, ObjFileName)) {
		std::cout << "Failed to Load Mesh Obj File\n";
		return;
	}
}

void Mesh::CreateVAO(std::vector<float>VertexData, std::vector<float>NormalData,
	std::vector<float>UVData, std::vector<glm::ivec3> FaceVec, std::string mtlName,
	VAO& vao)
{
	// Veretxの構造書き換え処理
	std::vector<unsigned int> newIndex(0);
	std::vector<float> newVertices(0);

	std::vector<glm::ivec3> TFaceVec(0);
	// FaceVecの重複indexを排除
	for (int i = 0; i < FaceVec.size(); i++) {
		bool IsHaving = false;
		for (int j = 0; j < TFaceVec.size(); j++) {
			if (FaceVec[i] == TFaceVec[j]) {
				IsHaving = true;
				break;
			}
		}
		if (!IsHaving) {
			TFaceVec.push_back(FaceVec[i]);
		}
	}

	// 新しいIndex作成
	for (int i = 0; i < FaceVec.size(); i++) {
		int Idx = -1;
		for (int j = 0; j < TFaceVec.size(); j++) {
			if (FaceVec[i] == TFaceVec[j]) {
				Idx = j;
				break;
			}
		}
		newIndex.push_back(Idx);
	}

	// TFaceVecに対応する vertices作成
	for (int i = 0; i < TFaceVec.size(); i++) {
		glm::ivec3 this_vec = TFaceVec[i];
		int vertI = 3 * (this_vec.x - 1);
		int uvI = 2 * (this_vec.y - 1);
		int normalI = 3 * (this_vec.z - 1);
		newVertices.push_back(VertexData[vertI]);
		newVertices.push_back(VertexData[vertI + 1]);
		newVertices.push_back(VertexData[vertI + 2]);
		newVertices.push_back(NormalData[normalI]);
		newVertices.push_back(NormalData[normalI + 1]);
		newVertices.push_back(NormalData[normalI + 2]);
		newVertices.push_back(UVData[uvI]);
		newVertices.push_back(UVData[uvI + 1]);
	}


	// VAO作成処理
	unsigned int VertexArray;
	unsigned int VertexBuffer;
	unsigned int IndexBuffer;

	mShader->UseProgram();
	glGenVertexArrays(1, &VertexArray);
	glBindVertexArray(VertexArray);

	glGenBuffers(1, &VertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, newVertices.size() * sizeof(float), newVertices.data(), GL_STATIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &IndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, newIndex.size() * sizeof(int), newIndex.data(), GL_STATIC_DRAW);

	// link attribution
	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	//glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// unbind cube vertex arrays
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	vao.VertexArray = VertexArray;
	vao.VertexBuffer = VertexBuffer;
	vao.IndexBuffer = IndexBuffer;
	vao.IndicesSize = newIndex.size();
	vao.MaterialName = mtlName;
}

bool Mesh::LoadVAO(FILE* file, VAO& vao, int& VertexOffset, int& NormalOffset, int& UVOffset)
{
	std::vector<float> VertexData;
	std::vector<float> NormalData;
	std::vector<float> UVData;
	std::vector<glm::ivec3> FaceVec;
	std::string mtlName;

	while (1) {
		char lineHeader[1000];
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF) {
			VertexOffset += VertexData.size() / 3;
			UVOffset += UVData.size() / 2;
			NormalOffset += NormalData.size() / 3;
			// VAO作成
			CreateVAO(VertexData, NormalData, UVData, FaceVec, mtlName, vao);

			return false;
		}

		if (strcmp(lineHeader, "v") == 0) {
			char x_str[20];
			char y_str[20];
			char z_str[20];
			fscanf(file, "%s %s %s", x_str, y_str, z_str);
			VertexData.push_back(atof(x_str));
			VertexData.push_back(atof(y_str));
			VertexData.push_back(atof(z_str));
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			char x_str[20];
			char y_str[20];
			fscanf(file, "%s %s", x_str, y_str);
			UVData.push_back(atof(x_str));
			UVData.push_back(atof(y_str));
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			char x_str[20];
			char y_str[20];
			char z_str[20];
			fscanf(file, "%s %s %sn", x_str, y_str, z_str);
			NormalData.push_back(atof(x_str));
			NormalData.push_back(atof(y_str));
			NormalData.push_back(atof(z_str));
		}
		else if (strcmp(lineHeader, "f") == 0) {
			glm::ivec3 vertexIndex;
			glm::ivec3 uvIndex;
			glm::ivec3 normalIndex;
			fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d",
				&vertexIndex.x, &uvIndex.x, &normalIndex.x,
				&vertexIndex.y, &uvIndex.y, &normalIndex.y,
				&vertexIndex.z, &uvIndex.z, &normalIndex.z
			);
			glm::ivec3 a1, a2, a3;
			a1[0] = vertexIndex.x - VertexOffset;	a1[1] = uvIndex.x - UVOffset;	a1[2] = normalIndex.x - NormalOffset;
			a2[0] = vertexIndex.y - VertexOffset;	a2[1] = uvIndex.y - UVOffset;	a2[2] = normalIndex.y - NormalOffset;
			a3[0] = vertexIndex.z - VertexOffset;	a3[1] = uvIndex.z - UVOffset;	a3[2] = normalIndex.z - NormalOffset;
			FaceVec.push_back(a1);
			FaceVec.push_back(a2);
			FaceVec.push_back(a3);
		}
		else if (strcmp(lineHeader, "usemtl") == 0) {	// mtl name
			char str[100];
			fscanf(file, "%s", str);
			mtlName = str;
		}
		else if (strcmp(lineHeader, "o") == 0) {
			VertexOffset += VertexData.size() / 3;
			UVOffset += UVData.size() / 2;
			NormalOffset += NormalData.size() / 3;
			// VAO作成
			CreateVAO(VertexData, NormalData, UVData, FaceVec, mtlName, vao);
			return true;
		}
	}

	return true;
}

bool Mesh::LoadObjFile(std::string FilePath, std::string ObjFileName)
{
	std::string ObjFilePath = FilePath + ObjFileName;
	std::string mtlFileName;

	// obj fileを読みだす
	FILE* file = fopen(ObjFilePath.c_str(), "r");
	if (file == NULL) {
		printf("error: failed to open mesh file\n");
		return false;
	}

	while (1) {
		char lineHeader[128];
		// 行の最初の文字列を読み込みます。
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF) {
			return false;
		}

		if (strcmp(lineHeader, "mtllib") == 0) {	// mtl file name
			char str[20];
			fscanf(file, "%s", str);
			mtlFileName = str;

			res = fscanf(file, "%s", lineHeader);
			if (strcmp(lineHeader, "o") == 0) {		// object本体読み出し
				int VertexOffset = 0;
				int NormalOffset = 0;
				int UVOffset = 0;
				VAO vao;
				while (LoadVAO(file, vao, VertexOffset, NormalOffset, UVOffset)) {
					mVAOs.push_back(vao);
				}
				mVAOs.push_back(vao);
				break;
			}
			else {
				return false;
			}
		}
	}

	// マテリアル読み出し
	LoadMaterials(FilePath, mtlFileName);


	return true;
}

bool Mesh::LoadMaterials(std::string FilePath, std::string MtlFileName)
{
	// Materialを読み込む
	std::string mtlPath = FilePath + MtlFileName;
	FILE* file = fopen(mtlPath.c_str(), "r");
	if (file == NULL) {
		printf("error: failed to open mesh file\n");
		return false;
	}


	bool IsNewMtl = false;
	bool IsFirstRead = true;

	Material mtl;
	std::string mtlName;
	mtl.tex = nullptr;
	while (1) {
		char lineHeader[1000];
		// 行の最初の文字列を読み込みます。
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF) {
			mtl.DirLight.direction = glm::vec3(0, 0, 1.0f);
			mMaterials.insert(std::make_pair(mtlName, mtl));
			break;
		}

		if (strcmp(lineHeader, "newmtl") == 0) {
			if (!IsFirstRead) {			// 初回以降はnewmtlをサインにmMaterialsの要素追加
				mtl.DirLight.direction = glm::vec3(0, 0, 1.0f);
				mMaterials.insert(std::make_pair(mtlName, mtl));
				mtl.tex = nullptr;
			}

			char str[200];
			fscanf(file, "%s", str);
			mtlName = str;
			IsFirstRead = false;
		}
		else if (strcmp(lineHeader, "Ns") == 0) {		// specular power
			char str[20];
			fscanf(file, "%s", str);
			mSpecPower = atof(str);
			mtl.SpecPower = atof(str);
		}
		else if (strcmp(lineHeader, "Ka") == 0) {	// Ambient Color
			char r[20];
			char g[20];
			char b[20];

			fscanf(file, "%s %s %s", r, g, b);
			mAmbientLightColor.x = atof(r);
			mAmbientLightColor.y = atof(g);
			mAmbientLightColor.z = atof(b);
			mtl.AmbientColor.x = atof(r);
			mtl.AmbientColor.y = atof(g);
			mtl.AmbientColor.z = atof(b);
		}
		else if (strcmp(lineHeader, "Kd") == 0) {	// Diffuse Color
			char r[20];
			char g[20];
			char b[20];

			fscanf(file, "%s %s %s", r, g, b);
			mDirectionalLight.diffuseColor.x = atof(r);
			mDirectionalLight.diffuseColor.y = atof(g);
			mDirectionalLight.diffuseColor.z = atof(b);
			mtl.DirLight.diffuseColor.x = atof(r);
			mtl.DirLight.diffuseColor.y = atof(g);
			mtl.DirLight.diffuseColor.z = atof(b);
		}
		else if (strcmp(lineHeader, "Ks") == 0) {	// Specular Color
			char r[20];
			char g[20];
			char b[20];

			fscanf(file, "%s %s %s", r, g, b);
			mDirectionalLight.specColor.x = atof(r);
			mDirectionalLight.specColor.y = atof(g);
			mDirectionalLight.specColor.z = atof(b);
			mtl.DirLight.specColor.x = atof(r);
			mtl.DirLight.specColor.y = atof(g);
			mtl.DirLight.specColor.z = atof(b);
		}
		else if (strcmp(lineHeader, "map_Kd") == 0) {
			char str[100];
			fscanf(file, "%s", str);
			std::string tex_path = FilePath + str;
			Texture* tex = new Texture(tex_path);
			mtl.tex = tex;
		}
	}

	fclose(file);
	return true;
}

void Mesh::SetMeshLightings()
{
	mShader->SetVectorUniform("uAmbientLight", mAmbientLightColor);
	mShader->SetVectorUniform("uDirLight.mDirection", mDirectionalLight.direction);
	mShader->SetVectorUniform("uDirLight.mDiffuseColor", mDirectionalLight.diffuseColor);
	mShader->SetVectorUniform("uDirLight.mSpecColor", mDirectionalLight.specColor);
	mShader->SetFloatUniform("uSpecPower", mSpecPower);
}

void Mesh::SetMeshTransforms()
{
	glm::mat4 ScaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(mMeshScale, mMeshScale, mMeshScale));
	glm::mat4 TranslateMat = glm::translate(glm::mat4(1.0f), mMeshPos);
	glm::mat4 TransformMat = TranslateMat * mMeshRotate * ScaleMat;

	mShader->SetMatrixUniform("uScale", ScaleMat);
	mShader->SetMatrixUniform("uTranslate", TranslateMat);
	mShader->SetMatrixUniform("uRotate", mMeshRotate);
}

void Mesh::BindVertexArray()
{
	glBindVertexArray(mVertexArray);
}

void Mesh::Draw()
{
	mShader->UseProgram();
	SetMeshTransforms();
	for (auto vao = mVAOs.begin(); vao != mVAOs.end(); vao++) {
		glBindVertexArray(vao->VertexArray);

		Material material = mMaterials[vao->MaterialName];
		// Set Lightings
		mShader->SetVectorUniform("uAmbientLight", material.AmbientColor);
		mShader->SetVectorUniform("uDirLight.mDirection", mLightDir);
		mShader->SetVectorUniform("uDirLight.mDiffuseColor", material.DirLight.diffuseColor);
		mShader->SetVectorUniform("uDirLight.mSpecColor", material.DirLight.specColor);
		mShader->SetFloatUniform("uSpecPower", material.SpecPower);

		// Set Texture
		if (material.tex != nullptr) {
			material.tex->BindTexture();
		}

		glDrawElements(GL_TRIANGLES, vao->IndicesSize, GL_UNSIGNED_INT, 0);

		if (material.tex != nullptr) {
			material.tex->UnBindTexture();
		}
	}
}
