#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include "shader.h"
#include "model.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "camera.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

const int windowWidth = 800;
const int windowHeight = 600;

// callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void updateGui();

Camera camera(glm::vec3(0.0f, 5.0f, 30.0f));
float lastX = windowWidth / 2.0f;
float lastY = windowHeight / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f; // 当前帧与上一帧的时间差
float lastFrame = 0.0f; // 上一帧的时间

float xRotation = -90;
float yRotation = 0;

static bool drawModel = true;
static bool drawMesh = false;
static bool drawSkeleton = false;
static float animationDuration = 10;

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
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	//开启深度测试
	glEnable(GL_DEPTH_TEST);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsClassic();//设置imgui样式
	ImGuiIO& io = ImGui::GetIO();
	//io.Fonts->AddFontFromFileTTF(u8"c:\\Windows\\Fonts\\微软雅黑.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
	
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");
	auto fonts = ImGui::GetIO().Fonts;
	//简体中文支持
	fonts->AddFontFromFileTTF(
		u8"c:\\Windows\\Fonts\\simhei.ttf",
		12.0f,
		NULL,
		fonts->GetGlyphRangesChineseFull()
	);


	Shader lineShader("./../resources/shaders/lineV.txt", "./../resources/shaders/lineF.txt");
	Shader shader("./../resources/shaders/vertext.txt", "./../resources/shaders/fragment.txt");
	Model model("./../resources/model.dae");
	model.playAnimation(true);

	while (!glfwWindowShouldClose(window)) {
		glClearColor(1.0, 1.0, 1.0, 0.0);
		glClearDepth(1.0);
		glPointSize(5);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// 帧时间差
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// 输入
		processInput(window);		
		
		glm::mat4 modelTrans = glm::mat4(1.0);
		modelTrans = glm::rotate(modelTrans, glm::radians(yRotation), glm::vec3(0.0, 1.0, 0.0));
		modelTrans = glm::rotate(modelTrans, glm::radians(xRotation), glm::vec3(1.0, 0.0, 0.0));
		glm::mat4 viewTrans = camera.GetViewMatrix();
		glm::mat4 projectionTrans = glm::perspective(glm::radians(camera.Zoom), (float)windowWidth / (float)windowHeight, 0.1f, 200.0f);

		shader.use();
		shader.setMat4("model", modelTrans);
		shader.setMat4("view", viewTrans);
		shader.setMat4("projection", projectionTrans);
		model.draw(shader, DRAW_ENTITY);
		shader.unuse();

		lineShader.use();
		lineShader.setMat4("model", modelTrans);
		lineShader.setMat4("view", viewTrans);
		lineShader.setMat4("projection", projectionTrans);
		model.draw(lineShader, DRAW_SKELETON);
		lineShader.unuse();

		updateGui();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();

	return 0;
}


void updateGui()
{
	ImGui::Begin("Demo");
	ImGui::Text(u8"渲染");
	ImGui::Checkbox(u8"模型", &drawModel);ImGui::SameLine();
	ImGui::Checkbox(u8"网格", &drawMesh);ImGui::SameLine();
	ImGui::Checkbox(u8"骨骼", &drawSkeleton);
	//ImGui::SameLine();
	ImGui::Text(u8"动画");
	ImGui::SliderFloat("", &animationDuration, 0, 100);
	ImGui::SameLine();
	ImGui::Button(u8"暂停");
	ImGui::End();
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// 每当窗口改变大小，GLFW会调用这个函数并填充相应的参数供你处理
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	// 返回这个按键是否正在被按下
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)//是否按下了返回键
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

// 	xRotation = ypos - windowHeight/2;
// 	if (xRotation > 90)
// 		xRotation = 90;
// 	else if (xRotation < -90)
// 		xRotation = -90;

	yRotation = xpos - windowWidth / 2;
	if (yRotation > 180)
		yRotation = 180;
	else if (yRotation < -180)
		yRotation = -180;

	//camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
