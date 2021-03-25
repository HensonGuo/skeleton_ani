#include "skeleton_drawer.h"
#include <iostream>

SkeletonDrawer::SkeletonDrawer()
{
}

void SkeletonDrawer::addBoneLine(vec3 start, uint startBoneId, vec3 end, uint endBoneId, vec4 color)
{
	Vertex startVertex;
	startVertex.position = start;
	startVertex.boneId = startBoneId;
	startVertex.color = color;
	vertices.push_back(startVertex);
	Vertex endVertex;
	endVertex.position = end;
	endVertex.boneId = endBoneId;
	endVertex.color = color;
	vertices.push_back(endVertex);
}

void SkeletonDrawer::draw(Shader& shader)
{
	glBindVertexArray(VAO);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(GL_LINES, 0, vertices.size());
	glDisable(GL_BLEND);
}

void SkeletonDrawer::setUp()
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
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, boneId));
	glBindVertexArray(0);
}

void SkeletonDrawer::clear()
{
	vertices.clear();
}
