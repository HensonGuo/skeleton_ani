#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "shader.h"

using namespace glm;

class LineDrawer {
public:
	LineDrawer();
	void addBoneLine(vec3 start, uint startBoneId, vec3 end, uint endBoneId, vec4 color);
	void draw(Shader& shader);
	void setUp();
	void setLineWidth(float lineWidth);
	void clear();
private:
	struct Vertex {
		vec3 position;
		vec4 color;
		uint boneId;
	};
	GLuint VAO;
	GLuint VBO;
	std::vector<Vertex> vertices;
	float lineWidth = 1.0f;
};

