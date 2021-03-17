#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

#include "shader.h"
#include "material.h"


using namespace std;
using namespace glm;


// ¶¥µã
struct Vertex {
	vec3 position;
	vec3 normal;
	vec2 texCoords;
	uvec4 boneIds = glm::uvec4(0);
	vec4 boneWeights = glm::vec4(-3.0f);
};

class Mesh
{
public:
	Mesh();
	Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Material*> &materials);

	void draw(Shader& shader);

private:
	//Mesh data
	vector<Vertex> vertices;
	vector<GLuint> indices;
	vector<Material*> materials;

	GLuint VAO;
	GLuint VBO;
	GLuint EBO;

	void SetupMesh();
};