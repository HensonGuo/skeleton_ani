#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>


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
	Mesh(vector<Vertex> vertices, vector<GLuint> indices, GLuint texture);

	void draw(GLuint shaders_program);

private:
	//Mesh data
	vector<Vertex> vertices;
	vector<GLuint> indices;
	GLuint texture;

	GLuint VAO;
	GLuint VBO;
	GLuint EBO;

	void SetupMesh();
};