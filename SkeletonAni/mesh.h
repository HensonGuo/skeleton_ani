#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "material.h"


using namespace std;
using namespace glm;


// ¶¥µã
struct Vertex {
	vec3 position;
	vec3 normal;
	vec2 uv;
	vec4 boneIds = glm::vec4(0);
	vec4 boneWeights = glm::vec4(0.0f);
};

class Mesh
{
public:
	Mesh();
	Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Material*> &materials);

	void draw(GLuint shaders_program);

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