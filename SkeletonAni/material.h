#pragma once

#include <vector>
#include <glad/glad.h>
#include <iostream>
#include <assimp/scene.h>
#include <string>
#include "stb_image.h"


using namespace std;


struct Texture
{
	GLuint id;
	aiString type;
	aiString path;
};


class Material {
public:
	vector<Texture> textures;
	Material(aiMaterial* mat, aiTextureType type, string type_name, const string directory);
	GLuint loadImage(const char* imagePath);
	void draw(GLuint shaders_program);
	void reset();
};