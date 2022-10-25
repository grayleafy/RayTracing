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

// RayTracerShader ������ţ�
// ����0��Framebuffer
// ����1��MeshVertex
// ����2��MeshFaceIndex

// ScreenShader ������ţ�
// ����0��Framebuffer

int main()
{
	// GLFW��ʼ��
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// ����GLFW����
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// �����¼�
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// ���ڲ�����꣬����ʾ���
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// �������е�OpenGL����ָ��
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	
	// CPU�������ʼ��
	CPURandomInit();

	//��Ļ֡�����ʼ��
	screen.init();
	screenBuffer.init(SCR_WIDTH, SCR_HEIGHT);

	//��ɫ��
	Shader RayTracerShader("RayTracerVertexShader.glsl", "RayTracerFragmentShader.glsl");
	Shader ScreenShader("ScreenVertexShader.glsl", "ScreenFragmentShader.glsl");
	
	//����ģ��
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

	//����BVH��
	BVHTree tree;
	tree.build(scene.meshes);
	tree.buildLinerTree();
//	tree.printLinearNode(0);
	

	//��������������
	getTexture(tree, RayTracerShader, ObjTex);
	//getTexture(scene.meshes, RayTracerShader, ObjTex);
	
	// ��Ⱦ��ѭ��
	while (!glfwWindowShouldClose(window))
	{
		// ����ʱ��
		tRecord.updateTime();
		// ����
		processInput(window);
		// ��Ⱦѭ����1
		cam.LoopIncrease();
		cout << "֡��ʱ: " << tRecord.deltaTime << endl;


		//����֡����
		{
			// �󶨵���ǰ֡������
			screenBuffer.setCurrentBuffer(cam.LoopNum);

			

			// ������ɫ��
			RayTracerShader.use();
			// screenBuffer�󶨵���������Ϊ����0��������������Ƭ����ɫ���е�historyTextureΪ����0
			RayTracerShader.setInt("historyTexture", 0);

			// �������ֵ��ֵ
			RayTracerShader.setFloat("randOrigin", 874264.0f * (GetCPURandom() + 1.0f));

			//����������Ϣ����
			ObjTex.bindTexBVH(RayTracerShader);
			//ObjTex.bindTex(RayTracerShader);
			
			// ���������ֵ
			RayTracerShader.setVec3("camera.camPos", cam.cameraPos);
			RayTracerShader.setVec3("camera.front", cam.cameraFront);
			RayTracerShader.setVec3("camera.right", cam.cameraRight);
			RayTracerShader.setVec3("camera.up", cam.cameraUp);
			RayTracerShader.setFloat("camera.halfH", cam.halfH);
			RayTracerShader.setFloat("camera.halfW", cam.halfW);
			RayTracerShader.setVec3("camera.leftbottom", cam.LeftBottomCorner);
			RayTracerShader.setInt("camera.LoopNum", cam.LoopNum);

			//����
			scene.setSphere(RayTracerShader);

			screen.DrawScreen();

			// ����Buffer
			//glfwSwapBuffers(window);
			//glfwPollEvents();
		}
		//��Ⱦ����Ļ
		{
			// �󶨵�Ĭ�ϻ�����
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			// ����
			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			ScreenShader.use();
			screenBuffer.setCurrentAsTexture(cam.LoopNum);
			// screenBuffer�󶨵���������Ϊ����0��������������Ƭ����ɫ���е�screenTextureΪ����0
			ScreenShader.setInt("screenTexture", 0);

			// ������Ļ
			screen.DrawScreen();

			//��һ��glactivetexture???

			// ����Buffer
			glfwSwapBuffers(window);
			glfwPollEvents();
		}

		
	}

	// ������ֹ
	glfwTerminate();


	return 0;
}

// ��������
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

// �����ڳߴ�仯, ������֡����
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
	cam.updateScreenRatio(SCR_WIDTH, SCR_HEIGHT);
	glViewport(0, 0, width, height);
}

// ����¼���Ӧ
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);
	cam.updateCameraFront(xpos, ypos);
}

// ����fov
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	cam.updateFov(static_cast<float>(yoffset));
}







