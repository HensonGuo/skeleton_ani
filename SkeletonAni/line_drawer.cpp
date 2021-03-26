#include "line_drawer.h"
#include <iostream>

LineDrawer::LineDrawer()
{
	vertices = new std::vector<Vertex>();

	glLineWidth(lineWidth);
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, color));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, index));
	glBindVertexArray(0);
}

void LineDrawer::addBoneLine(vec3 start, uint startId, vec3 end, uint endId, vec4 color)
{
	Vertex startVertex;
	startVertex.position = start;
	startVertex.index = startId;
	startVertex.color = color;
	vertices->push_back(startVertex);
	Vertex endVertex;
	endVertex.position = end;
	endVertex.index = endId;
	endVertex.color = color;
	vertices->push_back(endVertex);
}

void LineDrawer::draw(Shader& shader)
{
	glBindVertexArray(VAO);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices->size(), &(*vertices)[0], GL_STATIC_DRAW);
	glDrawArrays(GL_LINES, 0, vertices->size());
	glDisable(GL_BLEND);
}

void LineDrawer::setLineWidth(float lineWidth)
{
	lineWidth = lineWidth;
	glLineWidth(lineWidth);
}

void LineDrawer::clear()
{
	vertices->clear();
	delete vertices;
	vertices = new std::vector<Vertex>();
}
