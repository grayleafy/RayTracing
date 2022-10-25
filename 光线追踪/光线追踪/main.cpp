#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shader.h"
#include "Mesh.h"
#include "Model.h"
#include "Camera.h"
#include "RT_Screen.h"
#include "Tool.h"
#include "BVH.h"
#include "ObjectTexture.h"
#include "Scene.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

unsigned int SCR_WIDTH = 1200;
unsigned int SCR_HEIGHT = 800;

timeRecord tRecord;

Camera cam(SCR_WIDTH, SCR_HEIGHT);

RenderBuffer screenBuffer;
RT_Screen screen;
//BVHTree bvhTree;

ObjectTexture ObjTex;

// RayTracerShader 纹理序号：
// 纹理0：Framebuffer
// 纹理1：MeshVertex
// 纹理2：MeshFaceIndex

// ScreenShader 纹理序号：
// 纹理0：Framebuffer

int main()
{
	// GLFW初始化
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// 创建GLFW窗口
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// 交互事件
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// 窗口捕获鼠标，不显示鼠标
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// 加载所有的OpenGL函数指针
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	
	// CPU随机数初始化
	CPURandomInit();

	//屏幕帧缓冲初始化
	screen.init();
	screenBuffer.init(SCR_WIDTH, SCR_HEIGHT);

	//着色器
	Shader RayTracerShader("RayTracerVertexShader.glsl", "RayTracerFragmentShader.glsl");
	Shader ScreenShader("ScreenVertexShader.glsl", "ScreenFragmentShader.glsl");
	
	//加载模型
	Scene scene;
	//Model model("objects/first-cube.obj");
	Model model("objects/room.obj");
	Model cube("objects/cube.obj");
	//Model fox("objects/low-poly-fox-by-pixelmannen.obj");
	scene.pushModel(model, glm::vec3(5.0, 5.0, 5.0), glm::vec3(0.0,5.0,0.0));
	scene.pushModel(cube, glm::vec3(0.5, 5.0, -1.0), glm::vec3(0.0, 1.0, -2.0));
	//scene.pushModel(fox, glm::vec3(0.03, 0.03, 0.03), glm::vec3(-3.5, -2.4, 0.0));
	scene.pushSphere(Sphere(glm::vec3(0, 2.0, 1), 1.5, glm::vec3(0.9, 0.9, 0.9), 0.0, 0.0, 0));
	scene.pushSphere(Sphere(glm::vec3(-3, 2.0, 1), 1.5, glm::vec3(0.2, 0.2, 0.9), 0.0, 1.0, 0));

	//建立BVH树
	BVHTree tree;
	tree.build(scene.meshes);
	tree.buildLinerTree();
//	tree.printLinearNode(0);
	

	//设置三角形纹理
	getTexture(tree, RayTracerShader, ObjTex);
	//getTexture(scene.meshes, RayTracerShader, ObjTex);
	
	// 渲染大循环
	while (!glfwWindowShouldClose(window))
	{
		// 计算时间
		tRecord.updateTime();
		// 输入
		processInput(window);
		// 渲染循环加1
		cam.LoopIncrease();
		cout << "帧耗时: " << tRecord.deltaTime << endl;


		//处理帧缓冲
		{
			// 绑定到当前帧缓冲区
			screenBuffer.setCurrentBuffer(cam.LoopNum);

			

			// 激活着色器
			RayTracerShader.use();
			// screenBuffer绑定的纹理被定义为纹理0，所以这里设置片段着色器中的historyTexture为纹理0
			RayTracerShader.setInt("historyTexture", 0);

			// 随机数初值赋值
			RayTracerShader.setFloat("randOrigin", 874264.0f * (GetCPURandom() + 1.0f));

			//绑定三角形信息纹理
			ObjTex.bindTexBVH(RayTracerShader);
			//ObjTex.bindTex(RayTracerShader);
			
			// 相机参数赋值
			RayTracerShader.setVec3("camera.camPos", cam.cameraPos);
			RayTracerShader.setVec3("camera.front", cam.cameraFront);
			RayTracerShader.setVec3("camera.right", cam.cameraRight);
			RayTracerShader.setVec3("camera.up", cam.cameraUp);
			RayTracerShader.setFloat("camera.halfH", cam.halfH);
			RayTracerShader.setFloat("camera.halfW", cam.halfW);
			RayTracerShader.setVec3("camera.leftbottom", cam.LeftBottomCorner);
			RayTracerShader.setInt("camera.LoopNum", cam.LoopNum);

			//球体
			scene.setSphere(RayTracerShader);

			screen.DrawScreen();

			// 交换Buffer
			//glfwSwapBuffers(window);
			//glfwPollEvents();
		}
		//渲染到屏幕
		{
			// 绑定到默认缓冲区
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			// 清屏
			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			ScreenShader.use();
			screenBuffer.setCurrentAsTexture(cam.LoopNum);
			// screenBuffer绑定的纹理被定义为纹理0，所以这里设置片段着色器中的screenTexture为纹理0
			ScreenShader.setInt("screenTexture", 0);

			// 绘制屏幕
			screen.DrawScreen();

			//试一试glactivetexture???

			// 交换Buffer
			glfwSwapBuffers(window);
			glfwPollEvents();
		}

		
	}

	// 条件终止
	glfwTerminate();


	return 0;
}

// 按键处理
void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cam.ProcessKeyboard(FORWARD, tRecord.deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cam.ProcessKeyboard(BACKWARD, tRecord.deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cam.ProcessKeyboard(LEFT, tRecord.deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cam.ProcessKeyboard(RIGHT, tRecord.deltaTime);
}

// 处理窗口尺寸变化, 不兼容帧缓冲
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
	cam.updateScreenRatio(SCR_WIDTH, SCR_HEIGHT);
	glViewport(0, 0, width, height);
}

// 鼠标事件响应
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);
	cam.updateCameraFront(xpos, ypos);
}

// 设置fov
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	cam.updateFov(static_cast<float>(yoffset));
}







