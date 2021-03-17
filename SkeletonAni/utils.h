#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/scene.h>

typedef unsigned int uint;
typedef unsigned char byte;

inline glm::mat4 assimpToGlmMatrix(aiMatrix4x4 matrix) {
	glm::vec4 l1(matrix.a1, matrix.a2, matrix.a3, matrix.a4);
	glm::vec4 l2(matrix.b1, matrix.b2, matrix.b3, matrix.b4);
	glm::vec4 l3(matrix.c1, matrix.c2, matrix.c3, matrix.c4);
	glm::vec4 l4(matrix.d1, matrix.d2, matrix.d3, matrix.d4);

	return glm::mat4(l1, l2, l3, l4);
}

inline glm::vec3 assimpToGlmVec3(aiVector3D vec) {
	return glm::vec3(vec.x, vec.y, vec.z);
}

inline glm::quat assimpToGlmQuat(aiQuaternion quat) {
	glm::quat q;
	q.x = quat.x;
	q.y = quat.y;
	q.z = quat.z;
	q.w = quat.w;
	return q;
}