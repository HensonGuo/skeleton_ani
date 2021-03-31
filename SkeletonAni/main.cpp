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
int viewportX = 0;
int viewportY = 0;
int viewportWidth = 800;
int viewportHeight = 600;

// callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void cursor_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

void updateGui();

Camera camera(glm::vec3(0.0f, 5.0f, 30.0f));
float lastX = windowWidth / 2.0f;
float lastY = windowHeight / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f; // 当前帧与上一帧的时间差
float lastFrame = 0.0f; // 上一帧的时间

float xRotation = -0;
float yRotation = 0;

static bool drawModel = true;
static bool drawMesh = false;
static bool drawSkeleton = false;
static float animationDuration = 10;
static float animationElapsed = 0;

bool isRotationMode = false;

Model model;

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
	glfwSetCursorPosCallback(window, cursor_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	stbi_set_flip_vertically_on_load(true);//解决图像翻转问题，不需要在片段着色器的position设置为-y

	//开启深度测试
	glEnable(GL_DEPTH_TEST);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsClassic();//设置imgui样式
	
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


	Shader skeletonShader("./../resources/shaders/lineV.txt", "./../resources/shaders/lineF.txt");
	Shader modelShader("./../resources/shaders/vertext.txt", "./../resources/shaders/fragment.txt");
	
	model.loadModel("./../resources/dancing_vampire.dae");
	model.playAnimation(false);

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

		if (drawMesh)
		{
			glLineWidth(1);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		if (drawModel)
		{
			modelShader.use();
			modelShader.setMat4("model", modelTrans);
			modelShader.setMat4("view", viewTrans);
			modelShader.setMat4("projection", projectionTrans);
			model.draw(modelShader, DRAW_ENTITY);
			modelShader.unuse();
		}
		
		if (drawSkeleton)
		{
			skeletonShader.use();
			skeletonShader.setMat4("model", modelTrans);
			skeletonShader.setMat4("view", viewTrans);
			skeletonShader.setMat4("projection", projectionTrans);
			model.draw(skeletonShader, DRAW_SKELETON);
			skeletonShader.unuse();
		}

		animationDuration = model.getAniDuration();
		animationElapsed = model.getAniElapsed();
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
	ImGui::Text(u8"动画");

	if (ImGui::SliderFloat("", &animationElapsed, 0, animationDuration))
	{
		isRotationMode = false;
		model.changePoseStopAtTime(animationElapsed);
	}

	ImGui::SameLine();
	if (model.isPlayingAnimation())
	{
		if (ImGui::Button(u8"暂停"))
		{
			model.playAnimation(false);
		}
	}
	else
	{
		if (ImGui::Button(u8"播放"))
		{
			model.playAnimation(true);
		}
	}
	ImGui::End();
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// 每当窗口改变大小，GLFW会调用这个函数并填充相应的参数供你处理
	
	if (height == 0)
		height = 1;
	if ((float)width / (float)height > (float)windowWidth / (float)windowHeight)
	{
		viewportWidth = (float)windowWidth / (float)windowHeight * height;
		viewportHeight = height;
		viewportX = (width - viewportWidth) / 2;
		viewportY = 0;
	}
	else
	{
		viewportWidth = width;
		viewportHeight = (float)windowHeight / (float)windowWidth * width;
		viewportX = 0;
		viewportY = (height - viewportHeight) / 2;
	}
 	glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
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

void cursor_callback(GLFWwindow* window, double xpos, double ypos)
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



	if (isRotationMode)
	{
// 		xRotation = ypos - viewportHeight / 2;
// 		if (xRotation > 90)
// 			xRotation = 90;
// 		else if (xRotation < -90)
// 			xRotation = -90;

		yRotation = xpos - viewportWidth / 2;
		if (yRotation > 180)
			yRotation = 180;
		else if (yRotation < -180)
			yRotation = -180;
	}

	//camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT)
	{
		isRotationMode = true;
	}
	else if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT)
	{
		isRotationMode = false;
	}
}