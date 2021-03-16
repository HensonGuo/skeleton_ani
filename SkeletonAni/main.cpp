#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include "shader.h"
#include "model.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const int windowWidth = 800;
const int windowHeight = 600;

/*
骨骼动画 demo
*/
int main(int argc, char ** argv) {

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//主版本号
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//次版本号
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "SkeletonAni", NULL, NULL);
	if (window == NULL)
	{
		// 生成错误则输出错误信息
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	//开启深度测试
	glEnable(GL_DEPTH_TEST);

	Shader shader("./../resources/shaders/vertext.txt", "./../resources/shaders/fragment.txt");
	Model model("./../resources/model.dae");


	glm::mat4 projectionMatrix = glm::perspective(75.0f, (float)windowWidth / windowHeight, 0.01f, 100.0f);

	glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.2f, -5.0f)
		, glm::vec3(0.0f, .0f, 0.0f),
		glm::vec3(0, 1, 0));
	glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;

	glm::mat4 modelMatrix(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 1.0f, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(.2f, .2f, .2f));


	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();
		shader.setMat4("view_projection_matrix", viewProjectionMatrix);
		shader.setMat4("model_matrix", modelMatrix);
		model.draw(shader);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();

	return 0;
}