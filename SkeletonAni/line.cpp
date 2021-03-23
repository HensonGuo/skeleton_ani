#include "line.h"
#include <iostream>

Line::Line()
{
}

void Line::addLine(vec3 start, vec3 end, vec4 color)
{
	Vertex startVertex;
	startVertex.position = start;
	startVertex.color = color;
	vertices.push_back(startVertex);
	Vertex endVertex;
	endVertex.position = end;
	endVertex.color = color;
	vertices.push_back(endVertex);
}

void Line::draw(Shader& shader)
{
	glBindVertexArray(VAO);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(GL_LINES, 0, vertices.size());
	glDisable(GL_BLEND);
}

void Line::setUp()
{
	glLineWidth(6);
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, color));
	glBindVertexArray(0);
}
