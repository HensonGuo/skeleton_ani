#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/scene.h>

typedef unsigned int uint;
typedef unsigned char byte;

inline glm::mat4 assimpToGlmMatrix(aiMatrix4x4 mat) {
	glm::mat4 m;
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			m[x][y] = mat[y][x];
		}
	}
	return m;
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

inline GLFWwindow* initWindow(int &windowWidth,int &windowHeight) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//���汾��
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//�ΰ汾��
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "SkeletonAni", NULL, NULL);
	//�����ߣ�������������һ��GLFWwindow���ʵ����window
	if (window == NULL)
	{
		// ���ɴ��������������Ϣ
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(window);
	//glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	//glfwSetCursorPosCallback(window, mouse_callback);
	//glfwSetScrollCallback(window, scroll_callback);

	// ����GLFW����ϣ��ÿ�����ڵ�����С��ʱ����øı䴰�ڴ�С�ĺ���
	//glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	return window;
}