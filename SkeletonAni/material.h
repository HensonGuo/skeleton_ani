#pragma once

#include <vector>
#include <iostream>
#include <assimp/scene.h>
#include <string>

#include "stb_image.h"
#include "shader.h"


using namespace std;


struct Texture
{
	GLuint id;
	aiString type;
	aiString path;
};

static vector<Texture> texturesLoaded;

class Material {
public:
	Material(aiScene const* scene, aiMaterial* mat, aiTextureType type, string type_name, const string directory);
	GLuint loadImage(const char* imagePath);
	GLuint loadImageFromMemory(const aiTexture* texture);
	void draw(Shader& shader);
	void reset();
private:
	GLuint bind(unsigned char* data, int width, int height, int comp);
	vector<Texture> textures;
};