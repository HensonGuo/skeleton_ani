#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "shader.h"

using namespace glm;

class Line {
public:
	Line();
	void addLine(vec3 start, vec3 end, vec4 color);
	void draw(Shader& shader);
	void setUp();
private:
	struct Vertex {
		vec3 position;
		vec4 color;
	};
	GLuint VAO;
	GLuint VBO;
	std::vector<Vertex> vertices;
};

